#include <libethereum/ExtVM.h>
#include "VMConfig.h"
#include "VM.h"
using namespace std;
using namespace dev;
using namespace dev::eth;

void VM::copyDataToMemory(bytesConstRef _data, u256*_sp)
{
	auto offset = static_cast<size_t>(_sp[0]);
	s512 bigIndex = _sp[1];
	auto index = static_cast<size_t>(bigIndex);
	auto size = static_cast<size_t>(_sp[2]);

	size_t sizeToBeCopied = bigIndex + size > _data.size() ? _data.size() < bigIndex ? 0 : _data.size() - index : size;

	if (sizeToBeCopied > 0)
		std::memcpy(m_mem.data() + offset, _data.data() + index, sizeToBeCopied);
	if (size > sizeToBeCopied)
		std::memset(m_mem.data() + offset + sizeToBeCopied, 0, size - sizeToBeCopied);
}