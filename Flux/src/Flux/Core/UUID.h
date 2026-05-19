#pragma once


#include <cstdint>
#include <functional>

namespace Flux {

	class UUID final 
	{
	public:
		UUID();
		explicit UUID(uint64_t uuid);
		UUID(const UUID&) = default;
		UUID(std::nullptr_t uuid);

		operator uint64_t() const { return m_UUID; }
		bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
		bool operator!=(const UUID& other) const { return m_UUID != other.m_UUID; }

	private:
		uint64_t m_UUID;
	};

}

namespace std {
	template<>
	struct hash<Flux::UUID>
	{
		size_t operator()(const Flux::UUID& uuid) const
		{
			return hash<uint64_t>()(static_cast<uint64_t>(uuid));
		}
	};
}