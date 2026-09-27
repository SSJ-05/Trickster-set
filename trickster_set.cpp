// tricks and practices for low latency // 22.09.26 // ZeroK

#include <optional>
#include <variant>
#include <cstdlib>
#include <cstdint>
#include <string_view>
#include <iostream>
#include <type_traits>
#include <new>


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
struct TaggedNPC {

	enum class Type : std::uint8_t { Human, Beast, Robot, Object };

	Type type;	// tag - tells which member is active

	Human  human;
	Beast  beast;
	Robot  robot;
	Object object;
};


//////////////////////////////////////////////////////////////

// tagged union (UB for non active member)
struct UnionNPC {

	enum class Type : std::uint8_t { Human, Beast, Robot, Object };

	Type type;

	union Storage {

		Human  human;
		Beast  beast;
		Robot  robot;
		Object object;

		Storage() noexcept {}
		~Storage() noexcept {}
	} storage;

	UnionNPC ( std::string_view name,
		   std::string_view personality,
		   std::string_view clan,
		   std::uint16_t health ) noexcept
		: type ( Type::Human )
		{
			::new ( &storage.human )
				Human{ name, personality, clan, health };
		}

	~UnionNPC () noexcept 
	{
		switch ( type ) {

			case Type::Human  : storage.human.~Human(); break;
			case Type::Beast  : storage.beast.~Beast(); break;
			case Type::Robot  : storage.robot.~Robot(); break;
			case Type::Object : storage.object.~Object(); break;
		}
	}

};

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

		case UnionNPC::Type::Human  : return npc.storage.human.health;
		case UnionNPC::Type::Beast  : return npc.storage.beast.health;
		case UnionNPC::Type::Robot  : return npc.storage.robot.battery;
		case UnionNPC::Type::Object : return npc.storage.object.canInteract;
	}

	return 0;
}


[[ nodiscard ]]
inline
std::uint16_t  process_npc ( const std::variant<
		Human, Beast, Robot, Object>& npc ) noexcept {

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




int main () {

	using VariantNPC = std::variant<Human, Beast, Robot, Object>;

	std::cout << "\n\n";

	std::cout << "\n*** Size comparison ***\n";
	std::cout << "Tagged struct : " << sizeof( TaggedNPC ) << '\n';
	std::cout << "Tagged union  : " << sizeof( UnionNPC ) << '\n';
	std::cout << "Variant       : " << sizeof( VariantNPC ) << '\n';


	std::cout << "\n*** Alignment comparison ***\n";
	std::cout << "Tagged struct : " << alignof( TaggedNPC ) << '\n';
	std::cout << "Tagged union  : " << alignof( UnionNPC ) << '\n';
	std::cout << "Variant       : " << alignof( VariantNPC ) << '\n';



	std::cout << "\n*** Individual Size comparison ***\n";
	std::cout << "Human  : " << sizeof( Human ) << '\n';
	std::cout << "Beast  : " << sizeof( Beast ) << '\n';
	std::cout << "Robot  : " << sizeof( Robot ) << '\n';
	std::cout << "Object : " << sizeof( Object ) << '\n';


	std::cout << "\n*** Individual Alignment comparison ***\n";
	std::cout << "Human  : " << alignof( Human ) << '\n';
	std::cout << "Beast  : " << alignof( Beast ) << '\n';
	std::cout << "Robot  : " << alignof( Robot ) << '\n';
	std::cout << "Object : " << alignof( Object ) << '\n';





	std::cout << "\n\n";
	return EXIT_SUCCESS;
}
