// Implementation lifted from: https://github.com/martinus/unordered_dense (MIT license)

#pragma once
#include "dynamic_array.h"

namespace utility {
	namespace detail {
		inline void mum(u64* a, u64* b) {
#if defined(__SIZEOF_INT128__)
			__uint128_t r = *a;
			r *= *b;
			*a = static_cast<u64>(r);
			*b = static_cast<u64>(r >> 64U);
#elif defined(_MSC_VER) && defined(_M_X64)
			*a = _umul128(*a, *b, b);
#else
			u64 ha = *a >> 32U;
			u64 hb = *b >> 32U;
			u64 la = static_cast<u32>(*a);
			u64 lb = static_cast<u32>(*b);
			u64 hi{};
			u64 lo{};
			u64 rh = ha * hb;
			u64 rm0 = ha * lb;
			u64 rm1 = hb * la;
			u64 rl = la * lb;
			u64 t = rl + (rm0 << 32U);
			auto c = static_cast<u64>(t < rl);
			lo = t + (rm1 << 32U);
			c += static_cast<u64>(lo < t);
			hi = rh + (rm0 >> 32U) + (rm1 >> 32U) + c;
			*a = lo;
			*b = hi;
#endif
		}

		[[nodiscard]] inline auto mix(u64 a, u64 b) -> u64 {
			mum(&a, &b);
			return a ^ b;
		}

		[[nodiscard]] inline auto compute_hash(u64 x) -> u64 {
			return mix(x, UINT64_C(0x9E3779B97F4A7C15));
		}

		[[nodiscard]] inline auto r4(const u8* p) -> u64 {
			u32 v{};
			utility::memcpy(&v, p, 4);
			return v;
		}

		[[nodiscard]] inline auto r8(const u8* p) -> u64 {
			u64 v{};
			utility::memcpy(&v, p, 8U);
			return v;
		}

		[[nodiscard]] inline auto r3(const u8* p, u64 k) -> u64 {
			return (static_cast<u64>(p[0]) << 16U) | (static_cast<u64>(p[k >> 1U]) << 8U) | p[k - 1];
		}

		[[maybe_unused]] [[nodiscard]] inline auto compute_hash(void const* key, u64 len) -> u64 {
			static constexpr u64 secret[] = { 
				UINT64_C(0xa0761d6478bd642f),
				UINT64_C(0xe7037ed1a0b428db),
				UINT64_C(0x8ebc6af09c88c6e3),
				UINT64_C(0x589965cc75374cc3)
			};

			auto const* p = static_cast<u8 const*>(key);
			u64 seed = secret[0];
			u64 a;
			u64 b;

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
				u64 i = len;

				if((i > 48)) {
					u64 see1 = seed;
					u64 see2 = seed;

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

		/**
		 * \brief Base hash operator.
		 * \tparam type Type to hash
		 */
		template<typename type>
		struct hash {};

#define DETAIL_CREATE_HASH_OPERATOR(type)                      \
		template<>                                                 \
		struct hash<type> {                                        \
			auto operator()(const type& obj) const noexcept -> u64 { \
				return compute_hash(static_cast<u64>(obj));            \
			}                                                        \
		}

		DETAIL_CREATE_HASH_OPERATOR(i8);
		DETAIL_CREATE_HASH_OPERATOR(i16);
		DETAIL_CREATE_HASH_OPERATOR(i32);
		DETAIL_CREATE_HASH_OPERATOR(i64);

		DETAIL_CREATE_HASH_OPERATOR(u8);
		DETAIL_CREATE_HASH_OPERATOR(u16);
		DETAIL_CREATE_HASH_OPERATOR(u32);
		DETAIL_CREATE_HASH_OPERATOR(u64);

		DETAIL_CREATE_HASH_OPERATOR(f32);
		DETAIL_CREATE_HASH_OPERATOR(f64);

		DETAIL_CREATE_HASH_OPERATOR(char);
		DETAIL_CREATE_HASH_OPERATOR(bool);

		template<typename base_type>
		struct hash<base_type*> {
			auto operator()(const base_type* obj) const noexcept -> u64 {
				return compute_hash(reinterpret_cast<u64>(obj));
			}
		};

		// template<>
		// struct hash<std::string> {
		// 	auto operator()(const std::string& obj) const noexcept -> u64 {
		// 		return compute_hash(obj.data(), sizeof(char) * obj.get_size());
		// 	}
		// };
	} // namespace detail

	/**
	 * \brief Hash-based unordered map. Maps a \b key to a specific \b value.
	 * \tparam key Type to use as a key type
	 * \tparam value Type to use as the value type
	 * \tparam hash Hash to use when hashing the key type. A hash operator ("()") has to be implemented in order for the
	 * map to work correctly. Some basic hash operators are provided by default. 
	 * \tparam key_equal Key equality operator. Uses std::equal by default. 
	 */
	template<typename key, typename value, typename hash = detail::hash<key>, typename key_equal = std::equal_to<key>>
	class map {
		struct bucket {
			static constexpr u32 dist_inc = 1U << 8U;             // skip 1 byte fingerprint
			static constexpr u32 fingerprint_mask = dist_inc - 1; // mask for 1 byte of fingerprint

			u32 m_dist_and_fingerprint;                           // upper 3 byte: distance to original bucket. lower byte: fingerprint from hash
			u32 m_value_idx;                                      // index into the m_values vector.
		};
	public:
		using bucket_type = std::pair<key, value>;
		using bucket_container_type = dynamic_array<bucket_type>;

		using const_iterator = typename bucket_container_type::const_iterator;
		using iterator = typename bucket_container_type::iterator;

		using dist_and_fingerprint_type = decltype(bucket::m_dist_and_fingerprint);
		using value_idx_type = decltype(bucket::m_value_idx);
		using element_type = value;
		using key_type = key;

		map() : map(0) {}
		map(u64 bucket_count) : m_buckets(nullptr), m_num_buckets(0), m_max_bucket_capacity(0) {
			if(bucket_count != 0) {
				reserve(bucket_count);
			}
			else {
				allocate_buckets_from_shift();
				clear_buckets();
			}
		}

		map(std::initializer_list<bucket_type> ilist, u64 bucket_count = 0)
			: map(bucket_count) {
			for(const auto& i : ilist) {
				emplace(i);
			}
		}

		map(const map& other)
		: m_values(other.m_values), m_equal(other.m_equal), m_hash(other.m_hash) {
			copy_buckets(other);
		}
		map(map&& other) noexcept : m_buckets(nullptr) {
			*this = std::move(other);
		}
		~map() {
			utility::free(m_buckets);
		}

		auto operator=(const map& other) -> map& {
			if(&other != this) {
				deallocate_buckets();

				m_values = other.m_values;
				m_hash = other.m_hash;
				m_equal = other.m_equal;
				m_shifts = initial_shifts;
				copy_buckets(other);
			}

			return *this;
		}
		auto operator=(map&& other) noexcept -> map& {
			if(&other != this) {
				deallocate_buckets();

				m_values = std::move(other.m_values);
				other.m_values.clear();
				m_buckets = std::exchange(other.m_buckets, nullptr);
				m_num_buckets = std::exchange(other.m_num_buckets, 0);
				m_max_bucket_capacity = std::exchange(other.m_max_bucket_capacity, 0);
				m_shifts = std::exchange(other.m_shifts, initial_shifts);
				m_hash = std::exchange(other.m_hash, {});
				m_equal = std::exchange(other.m_equal, {});

				other.allocate_buckets_from_shift();
				other.clear_buckets();
			}
				
			return *this;
		}

		auto operator[](const key_type& k) -> element_type& {
			return try_emplace(k).first->second;
		}
		auto operator[](key_type&& k) -> element_type& {
			return try_emplace(std::move(k)).first->second;
		}

		auto insert(bucket_type&& v) -> std::pair<iterator, bool> {
			return emplace(std::move(v));
		}

		[[nodiscard]] auto at(const key_type& k) -> value& {
			return do_at(k);
		}

		[[nodiscard]] auto find(const key& k) const -> const_iterator {
			return do_find(k);
		}

		template<class... Args>
		auto emplace(Args&&... args) -> std::pair<iterator, bool> {
			auto& k = get_key(m_values.emplace_back(std::forward<Args>(args)...));
			auto h = mixed_hash(k);
			auto dist_and_fingerprint = dist_and_fingerprint_from_hash(h);
			auto bucket_idx = bucket_idx_from_hash(h);

			while(dist_and_fingerprint <= at(m_buckets, bucket_idx).m_dist_and_fingerprint) {
				if(
					dist_and_fingerprint == at(m_buckets, bucket_idx).m_dist_and_fingerprint &&
					m_equal(k, get_key(m_values[at(m_buckets, bucket_idx).m_value_idx]))
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

			auto value_idx = static_cast<value_idx_type>(m_values.get_size() - 1);

			if(is_full()) {
				increase_size();
			}
			else {
				place_and_shift_up({ dist_and_fingerprint, value_idx }, bucket_idx);
			}

			return { begin() + static_cast<u64>(value_idx), true };
		}

		void reserve(u64 capacity) {
			capacity = std::min(capacity, max_size());
			m_values.reserve(capacity);
			const auto shifts = calc_shifts_for_size(std::max(capacity, size()));

			if(0 == m_num_buckets || shifts < m_shifts) {
				m_shifts = shifts;

				deallocate_buckets();
				allocate_buckets_from_shift();
				clear_and_fill_buckets_from_values();
			}
		}

		[[nodiscard]] auto begin() noexcept -> iterator {
			return m_values.begin();
		}
		[[nodiscard]] auto begin() const noexcept -> const_iterator {
			return m_values.begin();
		}
		[[nodiscard]] auto cbegin() const noexcept -> const_iterator {
			return m_values.cbegin();
		}
		[[nodiscard]] auto end() noexcept -> iterator {
			return m_values.end();
		}
		[[nodiscard]] auto cend() const noexcept -> const_iterator {
			return m_values.cend();
		}
		[[nodiscard]] auto end() const noexcept -> const_iterator {
			return m_values.end();
		}

		[[nodiscard]] auto size() const noexcept -> u64 {
			return m_values.get_size();
		}
		[[nodiscard]] auto is_full() const -> bool {
			return size() > m_max_bucket_capacity;
		}
		[[nodiscard]] auto empty() const noexcept -> bool {
			return m_values.empty();
		}
	protected:
		template <class... Args>
		auto try_emplace(const key_type& k, Args&&... args) -> std::pair<iterator, bool> {
			return do_try_emplace(k, std::forward<Args>(args)...);
		}	

		auto do_at(const key_type& k) -> element_type& {
			if(auto it = find(k); (end() != it)) {
				return it->second;
			}
			 
			ASSERT(false, "key not found");
			return element_type{};
		}

		template <typename K, typename... Args>
		auto do_try_emplace(K&& k, Args&&... args) -> std::pair<iterator, bool> {
			auto h = mixed_hash(k);
			auto dist_and_fingerprint = dist_and_fingerprint_from_hash(h);
			auto bucket_idx = bucket_idx_from_hash(h);

			while(true) {
				auto* b = &at(m_buckets, bucket_idx);

				if(dist_and_fingerprint == b->m_dist_and_fingerprint) {
					if(m_equal(k, get_key(m_values[b->m_value_idx]))) {
						return { begin() + static_cast<u64>(b->m_value_idx), false };
					}
				}
				else if(dist_and_fingerprint > b->m_dist_and_fingerprint) {
					return do_place_element(
						dist_and_fingerprint,
						bucket_idx,
						std::piecewise_construct,
						std::forward_as_tuple(std::forward<K>(k)),
						std::forward_as_tuple(std::forward<Args>(args)...)
					);
				}

				dist_and_fingerprint = dist_inc(dist_and_fingerprint);
				bucket_idx = next(bucket_idx);
			}
		}

		template <typename... Args>
		auto do_place_element(dist_and_fingerprint_type dist_and_fingerprint, value_idx_type bucket_idx, Args&&... args) -> std::pair<iterator, bool> {
			m_values.emplace_back(std::forward<Args>(args)...);
			auto value_idx = static_cast<value_idx_type>(m_values.get_size() - 1);

			if(is_full()) {
				increase_size();
			}
			else {
				place_and_shift_up({ dist_and_fingerprint, value_idx }, bucket_idx);
			}

			return { begin() + static_cast<u64>(value_idx), true };
		}

		auto do_find(const key& k) const -> const_iterator {
			if(empty()) {
				return end();
			}

			auto mh = mixed_hash(k);
			auto dist_and_fingerprint = dist_and_fingerprint_from_hash(mh);
			auto bucket_idx = bucket_idx_from_hash(mh);
			auto* b = &at(m_buckets, bucket_idx);

			if(dist_and_fingerprint == b->m_dist_and_fingerprint && m_equal(k, get_key(m_values[b->m_value_idx]))) {
				return begin() + static_cast<u64>(b->m_value_idx);
			}

			dist_and_fingerprint = dist_inc(dist_and_fingerprint);
			bucket_idx = next(bucket_idx);
			b = &at(m_buckets, bucket_idx);

			if(dist_and_fingerprint == b->m_dist_and_fingerprint && m_equal(k, get_key(m_values[b->m_value_idx]))) {
				return begin() + static_cast<u64>(b->m_value_idx);
			}

			dist_and_fingerprint = dist_inc(dist_and_fingerprint);
			bucket_idx = next(bucket_idx);
			b = &at(m_buckets, bucket_idx);

			while(true) {
				if(dist_and_fingerprint == b->m_dist_and_fingerprint) {
					if(m_equal(k, get_key(m_values[b->m_value_idx]))) {
						return begin() + static_cast<u64>(b->m_value_idx);
					}
				}
				else if(dist_and_fingerprint > b->m_dist_and_fingerprint) {
					return end();
				}

				dist_and_fingerprint = dist_inc(dist_and_fingerprint);
				bucket_idx = next(bucket_idx);
				b = &at(m_buckets, bucket_idx);
			}
		}

		[[nodiscard]] static constexpr auto at(bucket* bucket_ptr, u64 offset) -> bucket& {
			return *(bucket_ptr + offset);
		}

		void increase_size() {
			if(m_max_bucket_capacity == max_bucket_count()) {
				m_values.pop_back();

				ASSERT(false, "map ran into an overflow");
				return;
			}

			--m_shifts;
			deallocate_buckets();
			allocate_buckets_from_shift();
			clear_and_fill_buckets_from_values();
		}

		void deallocate_buckets() {
			utility::free(m_buckets);
			m_buckets = nullptr;

			m_num_buckets = 0;
			m_max_bucket_capacity = 0;
		}

		void clear_and_fill_buckets_from_values() {
			clear_buckets();

			for(value_idx_type value_idx = 0, end_idx = static_cast<value_idx_type>(m_values.get_size()); value_idx < end_idx; ++value_idx) {
				auto const& k = get_key(m_values[value_idx]);
				auto [dist_and_fingerprint, b] = next_while_less(k);

				place_and_shift_up({ dist_and_fingerprint, value_idx }, b);
			}
		}

		template <typename K>
		[[nodiscard]] auto next_while_less(const K& k) const -> bucket {
			auto h = mixed_hash(k);
			auto dist_and_fingerprint = dist_and_fingerprint_from_hash(h);
			auto bucket_idx = bucket_idx_from_hash(h);

			while(dist_and_fingerprint < at(m_buckets, bucket_idx).m_dist_and_fingerprint) {
				dist_and_fingerprint = dist_inc(dist_and_fingerprint);
				bucket_idx = next(bucket_idx);
			}

			return { dist_and_fingerprint, bucket_idx };
		}

		[[nodiscard]] static constexpr auto dist_inc(dist_and_fingerprint_type x) -> dist_and_fingerprint_type {
			return static_cast<dist_and_fingerprint_type>(x + bucket::dist_inc);
		}

		[[nodiscard]] static constexpr auto get_key(const bucket_type& vt) -> const key_type& {
			return vt.first;
		}

		[[nodiscard]] auto next(value_idx_type bucket_idx) const -> value_idx_type {
			return bucket_idx + 1U == m_num_buckets ? 0 : static_cast<value_idx_type>(bucket_idx + 1U);
		}

		template<typename K>
		[[nodiscard]] constexpr auto mixed_hash(const K& k) const -> u64 {
			return detail::compute_hash(m_hash(k));
		}

		[[nodiscard]] constexpr auto dist_and_fingerprint_from_hash(u64 h) const -> dist_and_fingerprint_type {
			return bucket::dist_inc | (static_cast<dist_and_fingerprint_type>(h) & bucket::fingerprint_mask);
		}

		[[nodiscard]] constexpr auto bucket_idx_from_hash(u64 h) const -> value_idx_type {
			return static_cast<value_idx_type>(h >> m_shifts);
		}

		[[nodiscard]] static constexpr auto max_size() noexcept -> u64 {
			if constexpr((std::numeric_limits<value_idx_type>::max)() == (std::numeric_limits<u64>::max)()) {
				return u64{ 1 } << (sizeof(value_idx_type) * 8 - 1);
			}
			else {
				return u64{ 1 } << (sizeof(value_idx_type) * 8);
			}
		}

		[[nodiscard]] static constexpr auto calc_shifts_for_size(u64 s) -> u8 {
			auto shifts = initial_shifts;

			while(shifts > 0 && static_cast<u64>(static_cast<float>(calc_num_buckets(shifts)) * default_max_load_factor) < s) {
				--shifts;
			}

			return shifts;
		}

		static constexpr auto max_bucket_count() noexcept -> u64 {
			return max_size();
		}

		auto bucket_count() const noexcept -> u64 {
			return m_num_buckets;
		}

		void clear_buckets() {
			if(m_buckets != nullptr) {
				std::memset(&*m_buckets, 0, sizeof(bucket) * bucket_count());
			}
		}

		[[nodiscard]] static constexpr auto calc_num_buckets(u8 shifts) -> u64 {
			return std::min(max_bucket_count(), u64{ 1 } << (64U - shifts));
		}

		void place_and_shift_up(bucket b, value_idx_type place) {
			while(0 != at(m_buckets, place).m_dist_and_fingerprint) {
				b = std::exchange(at(m_buckets, place), b);
				b.m_dist_and_fingerprint = dist_inc(b.m_dist_and_fingerprint);
				place = next(place);
			}

			at(m_buckets, place) = b;
		}

		void allocate_buckets_from_shift() {
			m_num_buckets = calc_num_buckets(m_shifts);
			m_buckets = static_cast<bucket*>(utility::malloc(m_num_buckets * sizeof(bucket)));

			if(m_num_buckets == max_bucket_count()) {
				m_max_bucket_capacity = max_bucket_count();
			}
			else {
				m_max_bucket_capacity = static_cast<value_idx_type>(static_cast<float>(m_num_buckets) * default_max_load_factor);
			}
		}

		void copy_buckets(const map& other) {
			if(empty()) {
				allocate_buckets_from_shift();
				clear_buckets();
			}
			else {
				m_shifts = other.m_shifts;
				allocate_buckets_from_shift();
				utility::memcpy(m_buckets, other.m_buckets, sizeof(bucket) * bucket_count());
			}
		}
	protected:
		static constexpr f32 default_max_load_factor = 0.8f;
		static constexpr u8 initial_shifts = 64 - 2;

		bucket_container_type m_values;
		bucket* m_buckets;

		key_equal m_equal;
		hash m_hash;

		u64 m_num_buckets;
		u64 m_max_bucket_capacity;
		u8 m_shifts = initial_shifts;
	};
} // namespace utility
