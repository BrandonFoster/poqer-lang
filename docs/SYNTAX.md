# POQER syntax & constraints v0.001

# Starting Production
```
    <program> ::= <prolog-text>
```

# Prolog syntax & constraints that conforms to the ISO/IEC 13211-1
*A:* refers to the abstract,

*P:* refers to the priority,

*C:* refers to the conditions,

*S:* refers to the operator specifier

## Clauses and Directives
```
    <prolog-text> ::= <directive-term> <prolog-text>
A:  d*t               d                t       

    <prolog-text> ::= <clause-term> <prolog-text>
A:  c*t               c             t
    
    <prolog-text> ::= <EOR>
A:  nil

    <directive-term> ::= <term> <end>
A:  dt                   dt
P:                       1201
C:  The principal functor of dt is (:-)/1

    <clause-term> ::= <term> <end>
A:  c                 c
P:                    0
C:  The principal functor of c is not (:-)/1
```

## Numbers
```
    <term> ::= <integer>
A:  n          n
P:  0

    <term> ::= <float-number>
A:  r          r
P:  0

    <term> ::= <name> <integer>
A:  -n         a      n
P:  0
C:  a is the character -

    <term> ::= <name> <float-number>
A:  -r         a      r
P:  0
C:  a is the character -
```

## Atoms
```
    <term> ::= <atom>
A:  a          a
P:  0
C:  a is not an operator

    <term> ::= <atom>
A:  a          a
P:  1201
C:  a is an operator

    <atom> ::= <name>
A:  a          a

    <atom> ::= <open-list> <close-list>
A:  []

    <atom> ::= <open-curly> <close-curly>
A:  {}
```

## Variables
```
    <term> ::= <variable>
A:  v          v
P:  0
```
## Functional Notation
```
    <term> ::= <atom> <open-par> <arg-list> <close-par>
A:  f(l)       f                 l
P:  0

    <arg-list> ::= <arg>
A:  a              a

    <arg-list> ::= <arg> <comma> <arg-list>
A:  a,l            a             l


    <arg> ::= <atom>
A:  a         a
C:  a is an operator

    <arg> ::= <term>
A:  a         a
P:            999
```

## Operator Notation

### Operand
```
    <term> ::= <lterm>
A:  a          a
P:  n          n

    <lterm> ::= <term>
A:  a           a
P:  n           n-1

    <term> ::= <open-par> <term> <close-par>
A:  a                     a
P:  0                     1201
```

### Operators As Functors
```
    <lterm> ::= <term> <op> <term>
A:  f(a,b)      a      f    b
P:  n           n-1    n    n-1
S:                     xfx

    <lterm> ::= <lterm> <op> <term>
A:  f(a,b)      a       f    b
P:  n           n       n    n-1
S:                      yfx

    <term> ::= <term> <op> <term>
A:  f(a,b)      a      f    b
P:  n           n-1    n    n
S:                     xfy

    <lterm> ::= <lterm> <op>
A:  f(a)        a       f
P:  n           n       n
S:                      yf

    <lterm> ::= <term> <op>
A:  f(a)        a      f
P:  n           n-1    n
S:                     xf

    <term> ::= <op> <term>
A:  f(b)       f    b
P:  n          n    n
S:             fy
C: If b is a numeric constant, f is not -
C: The first token of b is not <open-par>

    <lterm> ::= <op> <term>
A:  f(b)        f    b
P:  n           n    n-1
S:              fx
C: If b is a numeric constant, f is not -
C: The first token of b is not <open-par>
```

### Operators
```
    <op> ::= <atom>
A:  a        a
P:  n        n
S:  s        s
C:  a is an operator

    <op> ::= <comma>
A:  ,
P:  1000
S:  xfy
```

## List Notation
```
    <term> ::= <open-list> <items> <close-list>
A:  l                      l
P:  0

    <items> ::= <arg> <comma> <items>
A:  .(h,l)      h             l

    <items> ::= <arg> <ht-sep> <arg>
A:  .(h,t)      h              t

    <items> ::= <arg>
A:  .(t,[])     t
```

## Curly-Bracketed Term
```
    <term> ::= <open-curly> <term> <close-curly>
A:  {}(l)                   l
P:  0                       1201
```

## Double-Quoted List Notation
```
    <term> ::= <double-quoted list>
A:  l          ccl
P:  0
```