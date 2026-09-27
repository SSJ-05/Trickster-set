// tricks and practices for low latency // 22.09.26 // ZeroK

#include <variant>
#include <cstdlib>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>
#include <benchmark/benchmark.h>




// forward decl of message types
struct Human {

	std::string_view name;
	std::string_view personality;
	std::string_view clan;

	std::uint16_t health;
};


struct Beast {

	std::string_view name;
	std::string_view species;

	std::uint16_t health;
};


struct Robot {

	std::string_view name;
	std::string_view firmware_version;

	std::uint16_t battery;	// equivalent to health
};


struct Object {

	std::string_view name;
	bool canInteract;
};


// tagged struct - all members always valid, no UB
struct alignas(64) TaggedNPC {

	enum class Type : std::uint8_t { Human, Beast, Robot, Object };

	Type type;	// tag - tells which member is active

	Human  human;
	Beast  beast;
	Robot  robot;
	Object object;
};


//////////////////////////////////////////////////////////////

// tagged union (UB for non active member)
struct alignas(64) UnionNPC {

	enum class Type : std::uint8_t { Human, Beast, Robot, Object };

	Type type;

	union {

		Human  human;
		Beast  beast;
		Robot  robot;
		Object object;
	};

};

using VariantNPC = std::variant<Human, Beast, Robot, Object>;

//////////////////////////////////////////////////////////////

// switch case
[[ nodiscard ]]
inline 
std::uint16_t  process_npc ( const TaggedNPC& npc ) noexcept {

	switch ( npc.type ) {

		case TaggedNPC::Type::Human  : return npc.human.health;
		case TaggedNPC::Type::Beast  : return npc.beast.health;
		case TaggedNPC::Type::Robot  : return npc.robot.battery;
		case TaggedNPC::Type::Object : return npc.object.canInteract;
	}

	return 0;
}


[[ nodiscard ]]
inline 
std::uint16_t  process_npc ( const UnionNPC& npc ) noexcept {

	switch ( npc.type ) {

		case UnionNPC::Type::Human  : return npc.human.health;
		case UnionNPC::Type::Beast  : return npc.beast.health;
		case UnionNPC::Type::Robot  : return npc.robot.battery;
		case UnionNPC::Type::Object : return npc.object.canInteract;
	}

	return 0;
}


[[ nodiscard ]]
inline
std::uint16_t  process_npc ( const VariantNPC& npc ) noexcept {

	return  std::visit( 
			[] ( const auto& x ) noexcept -> std::uint16_t {

				if constexpr ( std::is_same_v<
					std::decay_t<decltype(x)>, Human> ) {
						return  x.health;
				}
				else if constexpr ( std::is_same_v<
					std::decay_t<decltype(x)>, Beast> ) {
						return  x.health;
				}
				else if constexpr ( std::is_same_v<
					std::decay_t<decltype(x)>, Robot> ) {
						return  x.battery;
				}
				else return  x.canInteract;

			}, npc );
}




///////////////////////////////////////////////////////////////////////

// Object creation

// creates heterogeneous dataset array
template <typename T>
void  reserve_dataset ( std::vector<T>& npc, std::size_t n ) {

	npc.reserve( n );
}


// tagged_struct dataset generator
TaggedNPC  make_tagged_npc ( std::size_t i ) noexcept {

	switch ( i % 4 ) {

		case 0 : return  TaggedNPC {
				TaggedNPC::Type::Human,
				{ "ZeroK", "Cold", "Engineer", 100 },
				{},
				{},
				{}
			 };

		case 1 : return  TaggedNPC {
				TaggedNPC::Type::Beast,
				{},
				{ "ZeeK", "Wolf", 90 },
				{},
				{}
			 };

		case 2 : return  TaggedNPC {
				TaggedNPC::Type::Robot,
				{},
				{},
				{ "T-100", "v8.1", 200 },
				{}
			 };

		default : return  TaggedNPC {
				TaggedNPC::Type::Object,
				{},
				{},
			 	{},
				{ "Rock", false }
			 };
	}
}

// variant dataset generator
VariantNPC  make_variant_npc ( std::size_t i ) noexcept {

	switch ( i % 4 ) {

		case 0 : return  Human { "ZeroK", "Dark", "Architect", 110 };
		case 1 : return  Beast { "Zeek", "Dog", 80 };
		case 2 : return  Robot { "R2D2", "v8.0", 150 };
		default : return  Object { "Chair", true };
	}
}


///////////////////////////////////////////////////////////////////

// benchmarking

static void BM_tagged_struct ( benchmark::State& state ) {

	const std::size_t N { static_cast<std::size_t>( state.range( 0 ) ) };

	state.PauseTiming();

	std::vector<TaggedNPC> npc;
	npc.reserve( N );

	for ( auto i {0uz}; i < N; ++i ) {

		npc.emplace_back( make_tagged_npc( i ) );
	}

	state.ResumeTiming();

	for ( auto _ : state ) {

		auto sum { 0uz };
		for ( const auto& it : npc ) sum += process_npc( it );
		benchmark::DoNotOptimize( sum );
	}

	state.SetItemsProcessed(
			static_cast<std::size_t>( state.iterations() ) *
			static_cast<std::size_t>( N )
	);
}


// static void BM_union ( benchmark::State& state ) {
//
// 	UnionNPC npc {
// 		UnionNPC::Type::Human, { "ZeroK", "Dark", "Raven", 120 }
// 	};
// 	for ( auto _ : state ) {
// 		auto result = process_npc ( npc );
// 		benchmark::DoNotOptimize( result );
// 	}
// }


static void BM_variant ( benchmark::State& state ) {

	const std::size_t N { static_cast<std::size_t>( state.range( 0 ) ) };

	state.PauseTiming();

	std::vector<VariantNPC> npc;
	npc.reserve( N );

	for ( auto i {0uz}; i < N; ++i ) { 
		npc.emplace_back( make_variant_npc(i) );
	}

	state.ResumeTiming();

	for ( auto _ : state ) {
		auto sum { 0uz };
		for ( const auto& it : npc ) {
			sum += process_npc( it );
		}
		benchmark::DoNotOptimize( sum );
	}	

	state.SetItemsProcessed(
			static_cast<std::size_t>( state.iterations() ) *
			static_cast<std::size_t>( N )
	);
}


BENCHMARK ( BM_tagged_struct )
	-> RangeMultiplier( 4 )
	-> Range( 16, 1 << 20 );

// BENCHMARK ( BM_union );

BENCHMARK ( BM_variant )
	-> RangeMultiplier( 4 )
	-> Range( 16, 1 << 20 );


BENCHMARK_MAIN ();


