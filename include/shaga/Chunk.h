/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_Chunk
#define HEAD_shaga_Chunk

#include "common.h"

namespace shaga {
	class ChunkTool;

	static constexpr uint32_t _chunk_key_to_bin_helper (const char str[5], const size_t pos)
	{
		return (pos < 4) ? (_chunk_key_to_bin_helper (str, pos + 1) * 26) + (str[pos] - 'A')
						 : 0;
	}

	static constexpr uint32_t _chunk_key_to_bin (const char str[5])
	{
		return (str[0] >= 'A' && str[0] <= 'Z') &&
					   (str[1] >= 'A' && str[1] <= 'Z') &&
					   (str[2] >= 'A' && str[2] <= 'Z') &&
					   (str[3] >= 'A' && str[3] <= 'Z') &&
					   (str[4] == '\0')
				   ? (_chunk_key_to_bin_helper (str, 0) + 1)
				   : 0;
	}

	#define ChKEY(a) ([] () constexpr -> uint32_t {                                       \
		static_assert ((a)[0] >= 'A' && (a)[0] <= 'Z', "Invalid character in chunk key"); \
		static_assert ((a)[1] >= 'A' && (a)[1] <= 'Z', "Invalid character in chunk key"); \
		static_assert ((a)[2] >= 'A' && (a)[2] <= 'Z', "Invalid character in chunk key"); \
		static_assert ((a)[3] >= 'A' && (a)[3] <= 'Z', "Invalid character in chunk key"); \
		static_assert ((a)[4] == '\0', "Chunk key must be exactly 4 characters long");    \
		return shaga::_chunk_key_to_bin (a);                                              \
	}())

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

			static const constexpr TrustLevel _TrustLevel_first{TrustLevel::INTERNAL};
			static const constexpr TrustLevel _TrustLevel_last{TrustLevel::UNTRUSTED};

			enum class Priority : uint32_t {
				pCRITICAL = 0,
				pMANDATORY,
				pOPTIONAL,
				pDEBUG,
			};

			static const constexpr Priority _Priority_first{Priority::pCRITICAL};
			static const constexpr Priority _Priority_last{Priority::pDEBUG};

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

			static const constexpr TTL _TTL_first{TTL::TTL0};
			static const constexpr TTL _TTL_last{TTL::TTL7};

			enum class Channel : bool {
				PRIMARY = true,
				SECONDARY = false,
			};

			static const constexpr uint32_t key_type_min = ChKEY ("AAAA");
			static const constexpr uint32_t key_type_max = ChKEY ("ZZZZ");

			/* TRAC chunk has one extra capability. It counts number of hops independently of TTL value and it has only 16-bit header
			instead of 32-bit. It still retains all other capabilities standard chunk has like payload and metadata. */
			static const constexpr uint32_t key_type_tracert = ChKEY ("TRAC");

			static const uint32_t key_tracert_mask		{0b0000'0000'0000'1111'0000'0000'0000'0000};

			static const uint32_t key_type_mask			{0b0000'0000'0000'0111'1111'1111'1111'1111};
			static const uint32_t key_special_type_mask	{0b0000'0000'0000'1000'0000'0000'0000'0000};
			static const uint32_t key_trust_mask		{0b0000'0000'0011'0000'0000'0000'0000'0000};
			static const uint32_t key_prio_mask			{0b0000'0000'1100'0000'0000'0000'0000'0000};
			static const uint32_t key_ttl_mask			{0b0000'0111'0000'0000'0000'0000'0000'0000};

			static const uint32_t key_has_payload_mask	{0b0000'1000'0000'0000'0000'0000'0000'0000};
			static const uint32_t key_has_dest_mask		{0b0001'0000'0000'0000'0000'0000'0000'0000};
			static const uint32_t key_has_cbor_mask		{0b0010'0000'0000'0000'0000'0000'0000'0000};
			static const uint32_t key_channel_mask		{0b0100'0000'0000'0000'0000'0000'0000'0000};

			static const uint32_t key_highbit_mask		{0b1000'0000'0000'0000'0000'0000'0000'0000};

			static const constexpr uint32_t key_trust_shift = BIN::count_trailing_zeros (key_trust_mask);
			static const constexpr uint32_t key_prio_shift = BIN::count_trailing_zeros (key_prio_mask);
			static const constexpr uint32_t key_ttl_shift = BIN::count_trailing_zeros (key_ttl_mask);

			static const constexpr uint8_t max_ttl{0x7};
			static const constexpr uint8_t max_hop_counter{max_ttl};

			static const constexpr uint32_t num_special_types{0b111};

			static const constexpr uint_fast8_t channel_primary{0b01};
			static const constexpr uint_fast8_t channel_secondary{0b10};

			typedef std::array<uint_fast32_t, num_special_types> SPECIAL_TYPES;

			static uint32_t key_to_bin (const std::string_view key);
			static std::string bin_to_key (const uint32_t bkey);

		private:
			bool _channel{true};
			HWID _hwid_source{HWID_UNKNOWN};
			uint32_t _type{0};
			std::string _payload;
			std::vector<uint8_t> _cbor;
			Priority _prio{Priority::pMANDATORY};
			TrustLevel _trust{TrustLevel::INTERNAL};
			uint_fast64_t _counter{0};
			uint_fast8_t _ttl{max_ttl};
			HWIDMASK _hwid_dest;
			std::list<TRACERT_HOP> _tracert_hops;

			mutable std::string _stored_binary;
			mutable size_t _stored_header_size{0};

			bool should_continue (const std::string_view s, const size_t offset) const;
			uint32_t generate_header (char *const out, size_t &offset, const SPECIAL_TYPES *const special_types) const;

			void _reset (void);

			void _construct (void);

			template <typename... Types>
			void _construct (const std::string_view payload, Types &&...rest)
			{
				_payload.assign (payload);
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (const std::string &payload, Types &&...rest)
			{
				_payload.assign (payload);
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (std::string &&payload, Types &&...rest)
			{
				_payload = std::move (payload);
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (const Priority prio, Types &&...rest)
			{
				_prio = prio;
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (const TrustLevel trust, Types &&...rest)
			{
				_trust = trust;
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (const TTL ttl, Types &&...rest)
			{
				_ttl = +ttl;
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (const Channel channel, Types &&...rest)
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

			template <typename... Types>
			void _construct (const HWIDMASK &hwidmask, Types &&...rest)
			{
				_hwid_dest = hwidmask;
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (const ChunkMeta &m, Types &&...rest)
			{
				meta = m;
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (ChunkMeta &&m, Types &&...rest)
			{
				meta = std::move (m);
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (const nlohmann::json &data, Types &&...rest)
			{
				_cbor = nlohmann::json::to_cbor (data);
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (const std::vector<uint8_t> &cbor, Types &&...rest)
			{
				_cbor = cbor;
				_construct (rest...);
			}

			template <typename... Types>
			void _construct (std::vector<uint8_t> &&cbor, Types &&...rest)
			{
				_cbor = std::move (cbor);
				_construct (rest...);
			}

		public:
			/* Important note about store_binary_representation: This function is intended for message routers and other usages, where main content
			 * of the Chunk is not changed - payload, cbor, destinaton and meta. You can change TTL, source HWID, priority and trustlevel, these
			 * are stored in header, which is always regenerated. It is quite expensive to check meta for changes, so you need to make sure manually
			 * that when you change meta data (ChunkMeta), you invalidate stored binary representation by calling invalidate_stored_binary_representation ().
			 *
			 * Methods to_bin() never use stored binary representation.
			 *
			 * By calling setters for payload, cbor/json and destination, invalidate_stored_binary_representation() is automatically called. Only meta data
			 * are not checked.
			 */
			ChunkMeta meta;

			Chunk ();
			Chunk (const std::string_view bin, size_t &offset, const SPECIAL_TYPES *const special_types = nullptr, bool store_binary_representation = false);

			Chunk (const HWID hwid_source, const std::string_view type);
			Chunk (const HWID hwid_source, const uint32_t type);

			template <typename... Types>
			Chunk (const HWID hwid_source, const std::string_view type, Types &&...rest)
				: Chunk (hwid_source, type)
			{
				_construct (rest...);
			}

			template <typename... Types>
			Chunk (const HWID hwid_source, const uint32_t type, Types &&...rest)
				: Chunk (hwid_source, type)
			{
				_construct (rest...);
			}

			/* Returns number of bytes needed to store this chunk in binary format. May return more than actually needed but never less. */
			size_t get_max_bytes (void) const;

			/* Channel - primary / secondary */
			void set_channel (const bool is_primary);
			void set_channel (const Channel channel);
			bool is_primary_channel (void) const;
			bool is_secondary_channel (void) const;
			Channel get_channel (void) const;
			uint_fast8_t get_channel_bitmask (void) const;

			/* Return source HWID */
			HWID get_source_hwid (void) const;

			/* Check destination HWIDMASK against parameter */
			bool is_for_destination (const HWID hwid) const;
			bool is_for_destination (const HWID_LIST &lst) const;

			/* Type */
			std::string get_type (void) const;
			uint32_t get_num_type (void) const;

			/* Payload */
			bool has_payload (void) const;
			void reset_payload (void);

			void set_payload (const std::string_view payload);
			void set_payload (std::string &&payload);
			void swap_payload (std::string &other);

			template <typename T = std::string_view>
			SHAGA_STRV T get_payload (void) const
			{
				return _payload;
			}

			/* CBOR and JSON */
			bool has_cbor (void) const;
			void reset_cbor (void);

			void set_json (const nlohmann::json &data);
			void set_cbor (const std::vector<uint8_t> &cbor);
			void set_cbor (std::vector<uint8_t> &&cbor);
			void swap_cbor (std::vector<uint8_t> &other);

			nlohmann::json get_json (void) const;
			const std::vector<uint8_t> &get_cbor (void) const;

			/* Priority */
			void set_prio (const Priority prio);
			Priority get_prio (void) const;
			SHAGA_STRV std::string_view get_prio_text (void) const;

			/* Trustlevel */
			void set_minimal_trustlevel (const Chunk::TrustLevel trust);
			void set_trustlevel (const Chunk::TrustLevel trust);
			TrustLevel get_trustlevel (void) const;
			SHAGA_STRV std::string_view get_trustlevel_text (void) const;
			bool check_maximal_trustlevel (const Chunk::TrustLevel trust) const;

			/* TTL */
			void set_ttl (const uint8_t ttl);
			void set_ttl (const TTL ttl);
			uint8_t get_ttl (void) const;
			bool is_zero_ttl (void) const;
			bool hop_ttl (void);

			/* Tracert */
			bool tracert_hops_add (const shaga::HWID hwid, const uint8_t metric);
			std::list<TRACERT_HOP> tracert_hops_get (void) const;
			size_t tracert_hops_count (void) const;

			/* Destination */
			HWIDMASK get_destination_hwidmask (void) const;
			void set_destination_hwid (const HWID hwid);
			void set_destination_hwid (const HWIDMASK &hwidmask);
			void set_destination_broadcast (void);

			int compare (const Chunk &c) const;

			/* This method uses stored binary version or generates new version if content changed. Header is updated. */
			/* IMPORTANT: It has no way of checking if meta data changed. Don't use this method if you don't control all
			 * aspects of Chunk life cycle. Read more above. */
			void restore_bin (std::string &out, const bool do_swap, const SPECIAL_TYPES *const special_types = nullptr) const;

			/* Invalidate stored binary representation */
			void invalidate_stored_binary_representation (void) const;

			/* Returns stored binary representation or empty string_view is it is not valid. HEADER IS NOT UPDATED! */
			SHAGA_STRV std::string_view get_stored_binary_representation (void) const;

			/* These methods (to_bin) don't use stored binary version, they always generate output */
			void to_bin (std::string &out_append, const SPECIAL_TYPES *const special_types = nullptr) const;
			std::string to_bin (const SPECIAL_TYPES *const special_types = nullptr) const;

			friend bool operator== (const Chunk &a, const Chunk &b);
			friend bool operator!= (const Chunk &a, const Chunk &b);
			friend bool operator< (const Chunk &a, const Chunk &b);

			friend ChunkTool;
	};
}  // namespace shaga

#endif	// HEAD_shaga_Chunk
