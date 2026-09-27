# Runtime Polymorphic Dispatch
alternative practices worth giving a shot


# Tagged Union vs std::variant vs Tagged Struct 
- i saw unions in embedded code, std::variant in C++ code. both were found unreliable in hot path for low latency.
- low latency systems need deterministic and predictable code.

- The tagged struct says: "Every NPC contains a Human, Beast, Robot and Object."
- The union says: "Every NPC contains storage capable of holding one of these."
- The variant says: "Every NPC contains exactly one of these alternatives, and the active alternative is tracked."

- A raw union is cheap storage, but once the members are non trivial types, we becomes responsible for object lifetime.
- lifetime/abstraction cost:
			  		  union : programmer managed
					variant : library managed
- default union/variant size would be the largest object size + padding (Human object in this case) in the union



# Platform config
- Linux x86/64. Compiler version GCC 16.2


# Experiment 1 : size and alignment comparison

*** Size comparison ***
- Tagged struct : 168
- Tagged union  : 64
- Variant       : 64


Observations: 
1. struct with all active members had the maximum size (3x times union and variant)
2. with the current config/platform variant and union occupy the same space
3. union and variant can easily fit into a single 64-byte cache line, while a struct is larger than two cache lines
- Note: this doesn't prove smaller objects are cache friendly and therefore faster (please see Experiment 2)



# Experiment 2 : memory footprint




# Note:
1. I use std::string_view here for clarity and zero copy semantics. In hot path we might use  const char*  for literals or a custom "PackedString" for network buffers.
   The dispatch pattern is the point, the string type is interchangeable

