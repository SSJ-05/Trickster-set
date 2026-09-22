# Trickster-set
alternative practices worth giving a shot

# Tagged Union vs (Tagged Struct + Switch) 
i saw unions in embedded code, std::variant in C++ code. both were found unreliable in hot path for low latency.
low latency systems need deterministic and predictable code.


# Note: 
The following example uses std::cout for clarity. In a production hot path, 
you'd replace this with a non-blocking write() to a pre-allocated buffer,
or a custom logging macro that compiles to nothing in release builds. 
The dispatch pattern — the switch on the tag — is the key takeaway here, not the I/O.

