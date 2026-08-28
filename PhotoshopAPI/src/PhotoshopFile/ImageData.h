#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"
#include "Core/FileIO/Write.h"
#include "Core/Compression/Compress_RLE.h"

#include "blosc2.h"

#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

PSAPI_NAMESPACE_BEGIN


namespace ImageDataImpl
{
	template <typename T>
	using channel_data = std::vector<std::vector<T>>;

	template <typename T>
	void writeCompressedData(File& document, const FileHeader& header, const channel_data<T>& channels)
	{
		if (header.m_Version == Enum::Version::Psd)
		{
			std::vector<std::vector<uint16_t>> scanlineSizes;
			std::vector<std::vector<uint8_t>> compressedData;
			scanlineSizes.reserve(channels.size());
			compressedData.reserve(channels.size());
			for (const auto& channel : channels)
			{
				auto channelCopy = channel;
				std::vector<uint16_t> channelScanlineSizes;
				compressedData.push_back(CompressRLEImageDataPsd(channelCopy, header.m_Width, header.m_Height, channelScanlineSizes));
				scanlineSizes.push_back(std::move(channelScanlineSizes));
			}

			// First write all the scanline sizes, then the compressed data
			for (const auto& channelScanlineSizes : scanlineSizes)
			{
				auto data = channelScanlineSizes;
				WriteBinaryArray<uint16_t>(document, std::move(data));
			}
			for (const auto& channelData : compressedData)
			{
				auto data = channelData;
				WriteBinaryArray<uint8_t>(document, std::move(data));
			}
		}
		else
		{
			std::vector<std::vector<uint32_t>> scanlineSizes;
			std::vector<std::vector<uint8_t>> compressedData;
			scanlineSizes.reserve(channels.size());
			compressedData.reserve(channels.size());
			for (const auto& channel : channels)
			{
				auto channelCopy = channel;
				std::vector<uint32_t> channelScanlineSizes;
				compressedData.push_back(CompressRLEImageDataPsb(channelCopy, header.m_Width, header.m_Height, channelScanlineSizes));
				scanlineSizes.push_back(std::move(channelScanlineSizes));
			}

			// First write all the scanline sizes, then the compressed data
			for (const auto& channelScanlineSizes : scanlineSizes)
			{
				auto data = channelScanlineSizes;
				WriteBinaryArray<uint32_t>(document, std::move(data));
			}
			for (const auto& channelData : compressedData)
			{
				auto data = channelData;
				WriteBinaryArray<uint8_t>(document, std::move(data));
			}
		}
	}

	template <typename T>
	void writeRawData(File& document, const FileHeader& header, std::vector<T>&& uncompressedData)
	{
		WriteBinaryArray<T>(document, uncompressedData);
	}
}


/// \brief This section is for interoperability with different software such as lightroom and holds a composite of all the layers
///
/// When no merged image is supplied, this section is filled with empty pixels using Rle compression. Photoshop unfortunately
/// requires the section to be present.
struct ImageData : public FileSection
{
	using channel_data_8 = ImageDataImpl::channel_data<uint8_t>;
	using channel_data_16 = ImageDataImpl::channel_data<uint16_t>;
	using channel_data_32 = ImageDataImpl::channel_data<float32_t>;
	using channel_data_variant = std::variant<std::monostate, channel_data_8, channel_data_16, channel_data_32>;

	/// Write the merged image data section. This section is unfortunately required.
	inline void write(File& document, const FileHeader& header)
	{
		// Compression marker, we default to RLE compression to reduce the size significantly. The way in which the scanlines are stored
		// is slightly different though. All the channels store their scanline sizes at the start of the ImageData section rather than
		// at the start of each channel
		WriteBinaryData<uint16_t>(document, 1u);
		if (header.m_Depth == Enum::BitDepth::BD_8)
		{
			write_typed<uint8_t>(document, header);
		}
		else if (header.m_Depth == Enum::BitDepth::BD_16)
		{
			write_typed<uint16_t>(document, header);
		}
		else if (header.m_Depth == Enum::BitDepth::BD_32)
		{
			write_typed<float32_t>(document, header);
		}
	}

	ImageData() = default;

	/// Initialize the ImageData with a given number of channels to write out. We do this rather than deducting
	/// from the header as the header counts alpha channels while this does not!
	ImageData(uint16_t numChannels) : m_NumChannels(numChannels) {};

	template <typename T>
	void set_data(ImageDataImpl::channel_data<T> data)
	{
		m_ChannelData = std::move(data);
	}

private:
	template <typename T>
	void write_typed(File& document, const FileHeader& header)
	{
		using typed_data = ImageDataImpl::channel_data<T>;
		const auto* stored = std::get_if<typed_data>(&m_ChannelData);
		if (stored == nullptr)
		{
			ImageDataImpl::writeCompressedData(
				document,
				header,
				typed_data(m_NumChannels, std::vector<T>(static_cast<uint64_t>(header.m_Width) * header.m_Height, T{})));
			return;
		}

		if (stored->size() != m_NumChannels)
		{
			throw std::invalid_argument("merged image channel count does not match the Photoshop document header");
		}

		const auto expected_size = static_cast<uint64_t>(header.m_Width) * header.m_Height;
		for (const auto& channel : *stored)
		{
			if (channel.size() != expected_size)
			{
				throw std::invalid_argument("merged image channel size does not match the Photoshop document dimensions");
			}
		}
		ImageDataImpl::writeCompressedData(document, header, *stored);
	}

	uint16_t m_NumChannels = 0u;
	channel_data_variant m_ChannelData{};
};




PSAPI_NAMESPACE_END
