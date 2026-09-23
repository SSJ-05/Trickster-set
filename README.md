# Trickster-set
alternative practices worth giving a shot


# Tagged Union vs (Tagged Struct + Switch) 
i saw unions in embedded code, std::variant in C++ code. both were found unreliable in hot path for low latency.
low latency systems need deterministic and predictable code.

The tagged struct says: "Every NPC contains a Human, Beast, Robot and Object."
The union says: "Every NPC contains storage capable of holding one of these."
The variant says: "Every NPC contains exactly one of these alternatives, and the active alternative is tracked."




# Platform config
Linux x86/64. Compiler version GCC 16.2


# Experiment 1 : size and alignment comparison

*** Size comparison ***
Tagged struct : 152
Tagged union  : 48
Variant       : 48


Observations: 
1. struct with all active members had the maximum size (3x times union and variant)
2. with the current config/platform variant and union occupy the same space
3. union and variant can easily fit into a single 64-byte cache line, while a struct is larger than two cache lines
Note: this doesn't prove smaller objects are cache friendly and therefore faster (please see Experiment 2)



# Experiment 2 : memory footprint




# Note: 
The following example uses std::cout for clarity. In a production hot path, 
you'd replace this with a non-blocking write() to a pre-allocated buffer,
or a custom logging macro that compiles to nothing in release builds. 
The dispatch pattern — the switch on the tag — is the key takeaway here, not the I/O.

