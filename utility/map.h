#pragma once
#include "dynamic_array.h"
#include <vector>
#include <array>

namespace utility {
	namespace wyhash {
		inline void mum(uint64_t* a, uint64_t* b) {
#    if defined(__SIZEOF_INT128__)
			__uint128_t r = *a;
			r *= *b;
			*a = static_cast<uint64_t>(r);
			*b = static_cast<uint64_t>(r >> 64U);
#    elif defined(_MSC_VER) && defined(_M_X64)
			*a = _umul128(*a, *b, b);
#    else
			uint64_t ha = *a >> 32U;
			uint64_t hb = *b >> 32U;
			uint64_t la = static_cast<uint32_t>(*a);
			uint64_t lb = static_cast<uint32_t>(*b);
			uint64_t hi{};
			uint64_t lo{};
			uint64_t rh = ha * hb;
			uint64_t rm0 = ha * lb;
			uint64_t rm1 = hb * la;
			uint64_t rl = la * lb;
			uint64_t t = rl + (rm0 << 32U);
			auto c = static_cast<uint64_t>(t < rl);
			lo = t + (rm1 << 32U);
			c += static_cast<uint64_t>(lo < t);
			hi = rh + (rm0 >> 32U) + (rm1 >> 32U) + c;
			*a = lo;
			*b = hi;
#    endif
		}

		[[nodiscard]] inline auto mix(uint64_t a, uint64_t b) -> uint64_t {
			mum(&a, &b);
			return a ^ b;
		}

		[[nodiscard]] inline auto hash(uint64_t x) -> uint64_t {
			return mix(x, UINT64_C(0x9E3779B97F4A7C15));
		}

		[[nodiscard]] inline auto r4(const uint8_t* p) -> uint64_t {
			uint32_t v{};
			std::memcpy(&v, p, 4);
			return v;
		}

		[[nodiscard]] inline auto r8(const uint8_t* p) -> uint64_t {
			uint64_t v{};
			std::memcpy(&v, p, 8U);
			return v;
		}

		[[nodiscard]] inline auto r3(const uint8_t* p, size_t k) -> uint64_t {
			return (static_cast<uint64_t>(p[0]) << 16U) | (static_cast<uint64_t>(p[k >> 1U]) << 8U) | p[k - 1];
		}

		[[maybe_unused]] [[nodiscard]] inline auto hash(void const* key, size_t len) -> uint64_t {
			static constexpr auto secret = std::array{ UINT64_C(0xa0761d6478bd642f),
																								UINT64_C(0xe7037ed1a0b428db),
																								UINT64_C(0x8ebc6af09c88c6e3),
																								UINT64_C(0x589965cc75374cc3) };

			auto const* p = static_cast<uint8_t const*>(key);
			uint64_t seed = secret[0];
			uint64_t a{};
			uint64_t b{};
			if((len <= 16)) {
				if((len >= 4)) {
					a = (r4(p) << 32U) | r4(p + ((len >> 3U) << 2U));
					b = (r4(p + len - 4) << 32U) | r4(p + len - 4 - ((len >> 3U) << 2U));
				}
				else if((len > 0)) {
					a = r3(p, len);
					b = 0;
				}
				else {
					a = 0;
					b = 0;
				}
			}
			else {
				size_t i = len;
				if((i > 48)) {
					uint64_t see1 = seed;
					uint64_t see2 = seed;
					do {
						seed = mix(r8(p) ^ secret[1], r8(p + 8) ^ seed);
						see1 = mix(r8(p + 16) ^ secret[2], r8(p + 24) ^ see1);
						see2 = mix(r8(p + 32) ^ secret[3], r8(p + 40) ^ see2);
						p += 48;
						i -= 48;
					} while((i > 48));
					seed ^= see1 ^ see2;
				}
				while((i > 16)) {
					seed = mix(r8(p) ^ secret[1], r8(p + 8) ^ seed);
					i -= 16;
					p += 16;
				}
				a = r8(p + i - 16);
				b = r8(p + i - 8);
			}

			return mix(secret[1] ^ len, mix(a ^ secret[1], b ^ seed));
		}

	} // namespace wyhash

	namespace detail {
		template<typename type>
		struct hash {};

		template<>
		struct hash<int> {
			auto operator()(const int& obj) const noexcept -> u64 {
				return wyhash::hash(static_cast<u64>(obj));
			}
		};

		template<>
		struct hash<std::string> {
			auto operator()(const std::string& obj) const noexcept -> u64 {
				return wyhash::hash(obj.data(), sizeof(char) * obj.size());
			}
		};
	}

	namespace bucket_type {
		struct standard {
			static constexpr uint32_t dist_inc = 1U << 8U;             // skip 1 byte fingerprint
			static constexpr uint32_t fingerprint_mask = dist_inc - 1; // mask for 1 byte of fingerprint

			uint32_t m_dist_and_fingerprint; // upper 3 byte: distance to original bucket. lower byte: fingerprint from hash
			uint32_t m_value_idx;            // index into the m_values vector.
		};
	}

	template<
		typename key,
		typename value,
		typename hash = detail::hash<key>,
		typename key_equal = std::equal_to<key>,
		typename bucket = bucket_type::standard
	>
	class map {
	public:
		using underlying_value_type = dynamic_array<std::pair<key, value>>;

		using const_iterator = typename underlying_value_type::const_iterator;
		using iterator = typename underlying_value_type::iterator;

		using key_type = key;
		using value_type = value;
		using bucket_type = typename underlying_value_type::value_type;

		using dist_and_fingerprint_type = decltype(bucket::m_dist_and_fingerprint);
		using value_idx_type = decltype(bucket::m_value_idx);

		map() : map(0) {}

		explicit map(size_t bucket_count) {
			if(bucket_count != 0) {
				reserve(bucket_count);
				throw std::exception("not implemented");
			}
			else {
				allocate_buckets_from_shift();
				clear_buckets();
			}
		}

		~map() {
			delete[] m_buckets;
		}

		void reserve(u64 capa) {
			capa = (std::min)(capa, max_size());

			m_values.reserve(capa);

			auto shifts = calc_shifts_for_size((std::max)(capa, size()));
			if(0 == m_num_buckets || shifts < m_shifts) {
				m_shifts = shifts;
				deallocate_buckets();
				allocate_buckets_from_shift();
				clear_and_fill_buckets_from_values();
			}
		}

		auto operator[](key const& key) -> value& {
			return try_emplace(key).first->second;
		}

		auto operator[](key&& key) -> value& {
			return try_emplace(std::move(key)).first->second;
		}

		[[nodiscard]] auto size() const noexcept -> size_t {
			return m_values.size();
		}

		[[nodiscard]] auto is_full() const -> bool {
			return size() > m_max_bucket_capacity;
		}

		[[nodiscard]] auto max_load_factor() const -> float {
			return m_max_load_factor;
		}

		auto insert(bucket_type&& value) -> std::pair<iterator, bool> {
			return emplace(std::move(value));
		}

		template<class... Args>
		auto emplace(Args&&... args) -> std::pair<iterator, bool> {
			// we have to instantiate the bucket_type to be able to access the key.
			// 1. emplace_back the object so it is constructed. 2. If the key is already there, pop it later in the loop.

			auto& key = get_key(m_values.emplace_back(std::forward<Args>(args)...));
			auto hash = mixed_hash(key);
			auto dist_and_fingerprint = dist_and_fingerprint_from_hash(hash);
			auto bucket_idx = bucket_idx_from_hash(hash);

			while(dist_and_fingerprint <= at(m_buckets, bucket_idx).m_dist_and_fingerprint) {
				if(
					dist_and_fingerprint == at(m_buckets, bucket_idx).m_dist_and_fingerprint &&
					m_equal(key, get_key(m_values[at(m_buckets, bucket_idx).m_value_idx]))
				) {
					m_values.pop_back();
					return {
						begin() + static_cast<u64>(at(m_buckets, bucket_idx).m_value_idx),
						false
					};
				}

				dist_and_fingerprint = dist_inc(dist_and_fingerprint);
				bucket_idx = next(bucket_idx);
			}

			// value is new, place the bucket and shift up until we find an empty spot
			auto value_idx = static_cast<value_idx_type>(m_values.size() - 1);
			if(is_full()) {
				// increase_size just rehashes all the data we have in m_values
				increase_size();
			}
			else {
				// place element and shift up until we find an empty spot
				place_and_shift_up({ dist_and_fingerprint, value_idx }, bucket_idx);
			}

			return { begin() + static_cast<u64>(value_idx), true };
		}

		// Helper to access bucket through pointer types
		[[nodiscard]] static constexpr auto at(bucket* bucket_ptr, size_t offset) -> bucket& {
			return *(bucket_ptr + offset);
		}

		auto at(key_type const& key) -> value& {
			return do_at(key);
		}
		
		auto do_at(key const& key) -> value& {
			 if(auto it = find(key); (end() != it)) {
			 	return it->second;
			 }
			 
			 std::cerr << "key not found\n";
		}

		auto find(key const& key) -> iterator {
			return do_find(key);
		}

		auto begin() noexcept -> iterator {
			return m_values.begin();
		}

		auto begin() const noexcept -> const_iterator {
			return m_values.begin();
		}

		auto cbegin() const noexcept -> const_iterator {
			return m_values.cbegin();
		}

		auto end() noexcept -> iterator {
			return m_values.end();
		}

		auto cend() const noexcept -> const_iterator {
			return m_values.cend();
		}

		auto end() const noexcept -> const_iterator {
			return m_values.end();
		}

		[[nodiscard]] auto empty() const noexcept -> bool {
			return m_values.empty();
		}
	protected:
		template <class... Args>
		auto try_emplace(key const& key, Args&&... args) -> std::pair<iterator, bool> {
			return do_try_emplace(key, std::forward<Args>(args)...);
		}

		template <typename K, typename... Args>
		auto do_try_emplace(K&& key, Args&&... args) -> std::pair<iterator, bool> {
			auto hash = mixed_hash(key);
			auto dist_and_fingerprint = dist_and_fingerprint_from_hash(hash);
			auto bucket_idx = bucket_idx_from_hash(hash);

			while(true) {
				auto* bucket = &at(m_buckets, bucket_idx);

				if(dist_and_fingerprint == bucket->m_dist_and_fingerprint) {
					if(m_equal(key, get_key(m_values[bucket->m_value_idx]))) {
						return { begin() + static_cast<u64>(bucket->m_value_idx), false };
					}
				}
				else if(dist_and_fingerprint > bucket->m_dist_and_fingerprint) {
					return do_place_element(dist_and_fingerprint,
						bucket_idx,
						std::piecewise_construct,
						std::forward_as_tuple(std::forward<K>(key)),
						std::forward_as_tuple(std::forward<Args>(args)...));
				}
				dist_and_fingerprint = dist_inc(dist_and_fingerprint);
				bucket_idx = next(bucket_idx);
			}
		}

		template <typename... Args>
		auto do_place_element(dist_and_fingerprint_type dist_and_fingerprint, value_idx_type bucket_idx, Args&&... args)
			-> std::pair<iterator, bool> {

			// emplace the new value. If that throws an exception, no harm done; index is still in a valid state
			m_values.emplace_back(std::forward<Args>(args)...);

			auto value_idx = static_cast<value_idx_type>(m_values.size() - 1);
			if(is_full()) {
				increase_size();
			}
			else {
				place_and_shift_up({ dist_and_fingerprint, value_idx }, bucket_idx);
			}

			// place element and shift up until we find an empty spot
			return { begin() + static_cast<u64>(value_idx), true };
		}

		auto do_find(key const& key) -> iterator {
			if(empty()) {
				return end();
			}

			auto mh = mixed_hash(key);
			auto dist_and_fingerprint = dist_and_fingerprint_from_hash(mh);
			auto bucket_idx = bucket_idx_from_hash(mh);
			auto* bucket = &at(m_buckets, bucket_idx);

			// unrolled loop. *Always* check a few directly, then enter the loop. This is faster.
			if(dist_and_fingerprint == bucket->m_dist_and_fingerprint && m_equal(key, get_key(m_values[bucket->m_value_idx]))) {
				return begin() + static_cast<u64>(bucket->m_value_idx);
			}
			dist_and_fingerprint = dist_inc(dist_and_fingerprint);
			bucket_idx = next(bucket_idx);
			bucket = &at(m_buckets, bucket_idx);

			if(dist_and_fingerprint == bucket->m_dist_and_fingerprint && m_equal(key, get_key(m_values[bucket->m_value_idx]))) {
				return begin() + static_cast<u64>(bucket->m_value_idx);
			}
			dist_and_fingerprint = dist_inc(dist_and_fingerprint);
			bucket_idx = next(bucket_idx);
			bucket = &at(m_buckets, bucket_idx);

			while(true) {
				if(dist_and_fingerprint == bucket->m_dist_and_fingerprint) {
					if(m_equal(key, get_key(m_values[bucket->m_value_idx]))) {
						return begin() + static_cast<u64>(bucket->m_value_idx);
					}
				}
				else if(dist_and_fingerprint > bucket->m_dist_and_fingerprint) {
					return end();
				}

				dist_and_fingerprint = dist_inc(dist_and_fingerprint);
				bucket_idx = next(bucket_idx);
				bucket = &at(m_buckets, bucket_idx);
			}
		}

		void increase_size() {
			if(m_max_bucket_capacity == max_bucket_count()) {
				// remove the value again, we can't push_back it!
				m_values.pop_back();
				throw std::overflow_error("overflo");
			}

			--m_shifts;
			deallocate_buckets();
			allocate_buckets_from_shift();
			clear_and_fill_buckets_from_values();
		}

		void deallocate_buckets() {
			if(nullptr != m_buckets) {
				delete[] m_buckets;
				m_buckets = nullptr;
			}
			m_num_buckets = 0;
			m_max_bucket_capacity = 0;
		}

		void clear_and_fill_buckets_from_values() {
			clear_buckets();
			for(value_idx_type value_idx = 0, end_idx = static_cast<value_idx_type>(m_values.size()); value_idx < end_idx;
				++value_idx) {
				auto const& key = get_key(m_values[value_idx]);
				auto [dist_and_fingerprint, bucket] = next_while_less(key);

				// we know for certain that key has not yet been inserted, so no need to check it.
				place_and_shift_up({ dist_and_fingerprint, value_idx }, bucket);
			}
		}

		template <typename K>
		[[nodiscard]] auto next_while_less(K const& key) const -> bucket {
			auto hash = mixed_hash(key);
			auto dist_and_fingerprint = dist_and_fingerprint_from_hash(hash);
			auto bucket_idx = bucket_idx_from_hash(hash);

			while(dist_and_fingerprint < at(m_buckets, bucket_idx).m_dist_and_fingerprint) {
				dist_and_fingerprint = dist_inc(dist_and_fingerprint);
				bucket_idx = next(bucket_idx);
			}
			return { dist_and_fingerprint, bucket_idx };
		}

		// use the dist_inc and dist_dec functions so that uint16_t types work without warning
		[[nodiscard]] static constexpr auto dist_inc(dist_and_fingerprint_type x) -> dist_and_fingerprint_type {
			return static_cast<dist_and_fingerprint_type>(x + bucket::dist_inc);
		}

		[[nodiscard]] static constexpr auto get_key(bucket_type const& vt) -> key_type const& {
			return vt.first;
		}

		[[nodiscard]] auto next(value_idx_type bucket_idx) const -> value_idx_type {
			return bucket_idx + 1U == m_num_buckets ? 0 : static_cast<value_idx_type>(bucket_idx + 1U);
		}

		template<typename K>
		[[nodiscard]] constexpr auto mixed_hash(K const& key) const -> u64 {
			return wyhash::hash(m_hash(key));
		}

		[[nodiscard]] constexpr auto dist_and_fingerprint_from_hash(uint64_t hash) const -> dist_and_fingerprint_type {
			return bucket::dist_inc | (static_cast<dist_and_fingerprint_type>(hash) & bucket::fingerprint_mask);
		}

		[[nodiscard]] constexpr auto bucket_idx_from_hash(uint64_t hash) const -> value_idx_type {
			return static_cast<value_idx_type>(hash >> m_shifts);
		}

		[[nodiscard]] static constexpr auto max_size() noexcept -> size_t {
			if constexpr((std::numeric_limits<value_idx_type>::max)() == (std::numeric_limits<size_t>::max)()) {
				return size_t{ 1 } << (sizeof(value_idx_type) * 8 - 1);
			}
			else {
				return size_t{ 1 } << (sizeof(value_idx_type) * 8);
			}
		}

		[[nodiscard]] constexpr auto calc_shifts_for_size(size_t s) const -> uint8_t {
			auto shifts = initial_shifts;
			while(shifts > 0 && static_cast<size_t>(static_cast<float>(calc_num_buckets(shifts)) * max_load_factor()) < s) {
				--shifts;
			}
			return shifts;
		}


		static constexpr auto max_bucket_count() noexcept -> size_t {
			return max_size();
		}


		auto bucket_count() const noexcept -> size_t { // NOLINT(modernize-use-nodiscard)
			return m_num_buckets;
		}

		void clear_buckets() {
			if(m_buckets != nullptr) {
				std::memset(&*m_buckets, 0, sizeof(bucket) * bucket_count());
			}
		}

		[[nodiscard]] static constexpr auto calc_num_buckets(uint8_t shifts) -> size_t {
			return (std::min)(max_bucket_count(), size_t{ 1 } << (64U - shifts));
		}

		void place_and_shift_up(bucket bucket, value_idx_type place) {
			while(0 != at(m_buckets, place).m_dist_and_fingerprint) {
				bucket = std::exchange(at(m_buckets, place), bucket);
				bucket.m_dist_and_fingerprint = dist_inc(bucket.m_dist_and_fingerprint);
				place = next(place);
			}
			at(m_buckets, place) = bucket;
		}

		void allocate_buckets_from_shift() {
			m_num_buckets = calc_num_buckets(m_shifts);
			m_buckets = new bucket[m_num_buckets];

			if(m_num_buckets == max_bucket_count()) {
				m_max_bucket_capacity = max_bucket_count();
			}
			else {
				m_max_bucket_capacity = static_cast<value_idx_type>(static_cast<float>(m_num_buckets) * max_load_factor());
			}
		}
	protected:
		underlying_value_type m_values{};
		size_t m_num_buckets = 0;
		size_t m_max_bucket_capacity = 0;
		float m_max_load_factor = 0.8F;
		bucket* m_buckets{};

		hash m_hash{};
		key_equal m_equal{};
		static constexpr uint8_t initial_shifts = 64 - 2;
		uint8_t m_shifts = initial_shifts; // 2^(64-m_shift) number of buckets
	};
} // namespace utility
