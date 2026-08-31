#include "Lr16TaggedBlock.h"

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Lr16TaggedBlock::read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const Signature signature, const uint16_t padding)
{
	m_Key = Enum::TaggedBlockKey::Lr16;
	m_Offset = offset;
	m_Signature = signature;
	uint64_t length = ExtractWidestValue<uint32_t, uint64_t>(ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version));
	length = RoundUpToMultiple<uint64_t>(length, padding);
	m_Length = length;
	m_Data.read(document, header, callback, document.getOffset(), true, std::get<uint64_t>(m_Length));
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Lr16TaggedBlock::write(File& document, const FileHeader& header, ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("Lr16").m_Value);

	// LayerInfo::write() starts with the PSD/PSB-sized length field. In an
	// Lr16 tagged block that field is also the tagged-block payload length, so
	// adding another scoped length marker here would shift the layer count and
	// corrupt the document.
	m_Data.write(document, header, callback);
}


PSAPI_NAMESPACE_END
