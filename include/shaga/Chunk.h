/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_Chunk
#define HEAD_shaga_Chunk

#include "common.h"

namespace shaga {

	typedef std::list <Chunk> CHUNKLIST;
	typedef std::multiset <Chunk> CHUNKSET;
	typedef std::pair <CHUNKSET::const_iterator, CHUNKSET::const_iterator> CHUNKSET_RANGE;

	static constexpr uint32_t _chunk_key_to_bin_helper (const char str[5], const size_t pos)
	{
		return (pos < 4) ?
				(_chunk_key_to_bin_helper (str, pos + 1) * 26) + (str[pos] - 'A')
			:
				0;
	}

	static constexpr uint32_t _chunk_key_to_bin (const char str[5])
	{
		return (str[0] >= 'A' && str[0] <= 'Z') &&
			(str[1] >= 'A' && str[1] <= 'Z') &&
			(str[2] >= 'A' && str[2] <= 'Z') &&
			(str[3] >= 'A' && str[3] <= 'Z') &&
			(str[4] == '\0') ?
				(_chunk_key_to_bin_helper (str, 0) + 1)
			:
				0;
	}

	#define ChKEY(a) shaga::_chunk_key_to_bin(a)

	class Chunk {
		public:
			typedef struct {
				shaga::HWID hwid;
				uint8_t metric;
			} TRACERT_HOP;

			enum class TrustLevel : uint32_t {
				INTERNAL = 0,
				TRUSTED,
				FRIEND,
				UNTRUSTED,
			};

			static const TrustLevel _TrustLevel_first {TrustLevel::INTERNAL};
			static const TrustLevel _TrustLevel_last {TrustLevel::UNTRUSTED};

			enum class Priority : uint32_t {
				pCRITICAL = 0,
				pMANDATORY,
				pOPTIONAL,
				pDEBUG,
			};

			static const Priority _Priority_first {Priority::pCRITICAL};
			static const Priority _Priority_last {Priority::pDEBUG};

			enum class TTL : uint8_t {
				TTL0 = 0,
				TTL1 = 1,
				TTL2 = 2,
				TTL3 = 3,
				TTL4 = 4,
				TTL5 = 5,
				TTL6 = 6,
				TTL7 = 7,
			};

			static const TTL _TTL_first {TTL::TTL0};
			static const TTL _TTL_last {TTL::TTL7};

			enum class Channel : bool {
				PRIMARY = true,
				SECONDARY = false,
			};

			static const constexpr uint32_t key_type_min = ChKEY ("AAAA");
			static const constexpr uint32_t key_type_max = ChKEY ("ZZZZ");

			/* TRAC chunk has one extra capability. It counts number of hops independently of TTL value and it has only 16-bit header
			instead of 32-bit. It still retains all other capabilities standard chunk has like payload and metadata. */
			static const constexpr uint32_t key_type_tracert = ChKEY ("TRAC");
			static const uint32_t key_tracert_hop_mask {0b0000'0000'0000'0111'0000'0000'0000'0000};

			static const uint32_t key_type_mask        {0b0000'0000'0000'0111'1111'1111'1111'1111};
			static const uint32_t key_is_tracert_mask  {0b0000'0000'0000'1000'0000'0000'0000'0000};
			static const uint32_t key_trust_mask       {0b0000'0000'0011'0000'0000'0000'0000'0000};
			static const uint32_t key_prio_mask        {0b0000'0000'1100'0000'0000'0000'0000'0000};
			static const uint32_t key_ttl_mask         {0b0000'0111'0000'0000'0000'0000'0000'0000};

			static const uint32_t key_has_payload_mask {0b0000'1000'0000'0000'0000'0000'0000'0000};
			static const uint32_t key_has_dest_mask    {0b0001'0000'0000'0000'0000'0000'0000'0000};

			static const uint32_t key_channel_mask     {0b0010'0000'0000'0000'0000'0000'0000'0000};

			static const uint32_t key_reserved_mask    {0b0100'0000'0000'0000'0000'0000'0000'0000};

			static const uint32_t key_highbit_mask     {0b1000'0000'0000'0000'0000'0000'0000'0000};

			static const constexpr uint32_t key_tracert_hop_shift = BIN::count_trailing_zeros (key_tracert_hop_mask);
			static const constexpr uint32_t key_trust_shift = BIN::count_trailing_zeros (key_trust_mask);
			static const constexpr uint32_t key_prio_shift = BIN::count_trailing_zeros (key_prio_mask);
			static const constexpr uint32_t key_ttl_shift = BIN::count_trailing_zeros (key_ttl_mask);

			static const uint8_t max_ttl {0x7};
			static const uint8_t max_hop_counter {0x7};

			static const uint_fast8_t channel_primary {0b10};
			static const uint_fast8_t channel_secondary {0b01};

			static uint32_t key_to_bin (const std::string &key);
			static std::string bin_to_key (const uint32_t bkey);

		private:
			bool _channel {true};
			HWID _hwid_source {0};
			uint32_t _type {0};
			std::string _payload;
			Priority _prio {Priority::pMANDATORY};
			TrustLevel _trust {TrustLevel::INTERNAL};
			uint_fast64_t _counter {0};
			uint_fast8_t _ttl {max_ttl};
			HWIDMASK _hwid_dest;
			std::list<TRACERT_HOP> _tracert_hops;

			bool should_continue (const std::string &s, const size_t offset) const;

			void _reset (void);

			void _construct (void);

			template<typename ... Types>
			void _construct (const std::string &payload, Types&& ... rest)
			{
				_payload.assign (payload);
				_construct (rest...);
			}

			template<typename ... Types>
			void _construct (std::string &&payload, Types&& ... rest)
			{
				_payload = std::move (payload);
				_construct (rest...);
			}

			template<typename ... Types>
			void _construct (const Priority prio, Types&& ... rest)
			{
				_prio = prio;
				_construct (rest...);
			}

			template<typename ... Types>
			void _construct (const TrustLevel trust, Types&& ... rest)
			{
				_trust = trust;
				_construct (rest...);
			}

			template<typename ... Types>
			void _construct (const TTL ttl, Types&& ... rest)
			{
				_ttl = std::underlying_type<Chunk::TTL>::type (ttl);
				_construct (rest...);
			}

			template<typename ... Types>
			void _construct (const Channel channel, Types&& ... rest)
			{
				switch (channel) {
					case Channel::PRIMARY:
						_channel = true;
						break;

					case Channel::SECONDARY:
						_channel = false;
						break;
				}
				_construct (rest...);
			}

			template<typename ... Types>
			void _construct (const ChunkMeta &m, Types&& ... rest)
			{
				meta = m;
				_construct (rest...);
			}

			template<typename ... Types>
			void _construct (ChunkMeta &&m, Types&& ... rest)
			{
				meta = std::move (m);
				_construct (rest...);
			}

		public:
			ChunkMeta meta;

			Chunk ();
			Chunk (const std::string &bin, size_t &offset);

			Chunk (const HWID hwid_source, const std::string &type);
			Chunk (const HWID hwid_source, const uint32_t type);

			template<typename ... Types>
			Chunk (const HWID hwid_source, const std::string &type, Types&& ... rest) : Chunk (hwid_source, type)
			{
				_construct (rest...);
			}

			template<typename ... Types>
			Chunk (const HWID hwid_source, const uint32_t type, Types&& ... rest) : Chunk (hwid_source, type)
			{
				_construct (rest...);
			}

			void set_channel (const bool is_primary);
			void set_channel (const Channel channel);
			bool is_primary_channel (void) const;
			bool is_secondary_channel (void) const;
			Channel get_channel (void) const;
			uint_fast8_t get_channel_bitmask (void) const;

			HWID get_source_hwid (void) const;
			bool is_for_me (const HWID hwid) const;
			bool is_for_me (const HWID_LIST &lst) const;

			std::string get_type (void) const;
			uint32_t get_num_type (void) const;

			void set_payload (const std::string &payload);
			void set_payload (std::string &&payload);
			void swap_payload (std::string &other);
			std::string get_payload (void) const;

			void set_prio (const Priority prio);
			Priority get_prio (void) const;
			std::string get_prio_text (void) const;

			void set_minimal_trustlevel (const Chunk::TrustLevel trust);
			void set_trustlevel (const Chunk::TrustLevel trust);
			TrustLevel get_trustlevel (void) const;
			std::string get_trustlevel_text (void) const;
			bool check_maximal_trustlevel (const Chunk::TrustLevel trust) const;

			void set_ttl (const uint8_t ttl);
			void set_ttl (const TTL ttl);
			uint8_t get_ttl (void) const;
			bool is_zero_ttl (void) const;
			bool hop_ttl (void);

			bool tracert_hops_add (const shaga::HWID hwid, const uint8_t metric);
			std::list<TRACERT_HOP> tracert_hops_get (void) const;
			size_t tracert_hops_count (void) const;

			void set_destination_hwid (const HWID hwid);
			void set_destination_hwid (const HWIDMASK &hwidmask);
			void set_destination_broadcast (void);

			int compare (const Chunk &c) const;

			void to_bin (std::string &out) const;
			std::string to_bin (void) const;

			friend bool operator== (const Chunk &a, const Chunk &b);
			friend bool operator!= (const Chunk &a, const Chunk &b);
			friend bool operator< (const Chunk &a, const Chunk &b);

			friend void chunklist_change_source_hwid (CHUNKLIST &lst, const HWID new_source_hwid, const bool replace_only_zero);
	};

	static const Chunk::TTL TTL_local {Chunk::TTL::TTL1};
	static const Chunk::TTL TTL_wide {Chunk::_TTL_last};

	Chunk::TrustLevel operator++ (Chunk::TrustLevel &x);
	Chunk::TrustLevel operator++ (Chunk::TrustLevel &x, int r);
	Chunk::TrustLevel operator* (Chunk::TrustLevel c);
	Chunk::TrustLevel begin (Chunk::TrustLevel r);
	Chunk::TrustLevel end (Chunk::TrustLevel r);
	Chunk::TrustLevel uint8_to_trustlevel (const uint8_t v);
	uint8_t trustlevel_to_uint8 (const Chunk::TrustLevel v);
	std::string trustlevel_to_string (const Chunk::TrustLevel level);
	Chunk::TrustLevel string_to_trustlevel (const std::string &str);

	Chunk::Priority uint8_to_priority (const uint8_t v);
	uint8_t priority_to_uint8 (const Chunk::Priority v);
	std::string priority_to_string (const Chunk::Priority prio);

	Chunk::TTL uint8_to_ttl (const uint8_t v);
	uint8_t ttl_to_uint8 (const Chunk::TTL v);

	Chunk::Channel bool_to_channel (const bool val);
	bool channel_to_bool (const Chunk::Channel channel);
	std::string channel_to_string (const Chunk::Channel channel);

	CHUNKLIST chunkset_extract_type (const CHUNKSET &cs, const std::string &type);
	size_t chunkset_count_type (const CHUNKSET &cs, const std::string &type);

	CHUNKLIST bin_to_chunklist (const std::string &s, size_t &offset);
	CHUNKLIST bin_to_chunklist (const std::string &s);

	/* Append to CHUNKLIST &cs */
	void bin_to_chunklist (const std::string &s, size_t &offset, CHUNKLIST &cs);
	/* Append to CHUNKLIST &cs */
	void bin_to_chunklist (const std::string &s, CHUNKLIST &cs);

	/* Append out std::string &out */
	void chunklist_to_bin (CHUNKLIST &cs, std::string &out, const size_t max_size = 0, const Chunk::Priority max_priority = Chunk::Priority::pDEBUG, const bool erase_skipped = false);
	std::string chunklist_to_bin (CHUNKLIST &cs, const size_t max_size = 0, const Chunk::Priority max_priority = Chunk::Priority::pDEBUG, const bool erase_skipped = false);

	CHUNKSET bin_to_chunkset (const std::string &s, size_t &offset);
	CHUNKSET bin_to_chunkset (const std::string &s);

	/* Append to CHUNKSET &cs */
	void bin_to_chunkset (const std::string &s, size_t &offset, CHUNKSET &cs);
	/* Append to CHUNKSET &cs */
	void bin_to_chunkset (const std::string &s, CHUNKSET &cs);

	/* Append out std::string &out */
	void chunkset_to_bin (CHUNKSET &cs, std::string &out, const size_t max_size = 0, const Chunk::Priority max_priority = Chunk::Priority::pDEBUG, const bool thr = true);
	std::string chunkset_to_bin (CHUNKSET &cs, const size_t max_size = 0, const Chunk::Priority max_priority = Chunk::Priority::pDEBUG, const bool thr = true);

	void chunkset_trim (CHUNKSET &cset, const size_t treshold_size, const Chunk::Priority treshold_prio);

	void chunklist_change_source_hwid (CHUNKLIST &lst, const HWID new_source_hwid, const bool replace_only_zero = false);
}

#endif // HEAD_shaga_Chunk
