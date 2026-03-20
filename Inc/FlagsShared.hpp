#ifndef FLAGS_SHARED_HPP
#define FLAGS_SHARED_HPP

// #define USE_5_DOF
#define USE_1_DOF

#if defined(USE_5_DOF) && defined(USE_1_DOF)
#error "Must choose either 5 DOF or 1 DOF, not both."
#endif

#if !defined(USE_5_DOF) && !defined(USE_1_DOF)
#error "Must choose either 5 DOF or 1 DOF."
#endif

#endif // FLAGS_SHARED_HPP