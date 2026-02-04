# Mathematical Description of the Relations Model

## Introduction

This document describes the mathematical formalization of the Relations Model (RM) in terms of set theory from the perspective of [Links Theory](https://habr.com/ru/articles/895896/).

The Relations Model is a metaprogramming language based on the concept of a triune entity. Unlike traditional programming approaches that use two basic concepts (data and functions), in RM everything is represented by a single concept — the entity.

## 1. Basic Definitions

### 1.1. Entity References

**Definition 1.1 (Reference Set).** Let L ⊆ ℕ₀ be the set of natural numbers used as unique identifiers (references) to entities.

```
L = {0, 1, 2, 3, ...} ⊆ ℕ₀
```

Each reference l ∈ L uniquely identifies an entity in the Relations Model.

### 1.2. Triplets and Duplets

**Definition 1.2 (Reference Vector).** The set of all reference vectors of length n ∈ ℕ₀:

```
Vₙ = Lⁿ
```

where Lⁿ is the Cartesian power of set L.

**Definition 1.3 (Duplet).** A duplet (ordered pair of references) is an element of the set:

```
D = L² = L × L = {(a, b) : a ∈ L, b ∈ L}
```

**Definition 1.4 (Triplet).** A triplet (tuple of length 3) is an element of the set:

```
T = L³ = L × L × L = {(s, r, o) : s ∈ L, r ∈ L, o ∈ L}
```

where:
- s — reference to subject
- r — reference to relation
- o — reference to object

### 1.3. Representation of Triplet as Nested Ordered Pairs

From the perspective of set theory, a triplet can be represented as nested ordered pairs:

```
(s, r, o) = ((s, r), o)
```

This corresponds to the representation of a tuple through a Kuratowski pair.

## 2. Relations Model as Four Interconnected Sets

### 2.1. Four Aspects of an Entity

In the Relations Model, each entity can act in four aspects (roles):

1. **E (Entity)** — entity as a whole
2. **R (Relation)** — entity in the role of relation (controller)
3. **S (Subject)** — entity in the role of subject (view)
4. **O (Object)** — entity in the role of object (model)

**Definition 2.1 (Relations Model).** The Relations Model RM is represented as a quadruple of interconnected sets of tuples of length 3:

```
RM = {E, O, R, S}
```

where:

```
R ⊆ (S × O) × E = {((s, o), e) : s ∈ S, o ∈ O, e ∈ E}
O ⊆ (E × R) × S = {((e, r), s) : e ∈ E, r ∈ R, s ∈ S}
S ⊆ (R × E) × O = {((r, e), o) : r ∈ R, e ∈ E, o ∈ O}
E ⊆ (O × S) × R = {((o, s), r) : o ∈ O, s ∈ S, r ∈ R}
```

### 2.2. Internal Structure of Links

The internal aspect (structure) of each entity type is defined through links:

```
E = SO    (entity is defined by subject-object pair)
R = OS    (relation is defined by object-subject pair)
O = ER    (object is defined by entity-relation pair)
S = RE    (subject is defined by relation-entity pair)
```

### 2.3. Link Types (Duplets)

We define four basic link types:

```
OS ⊆ O × S = {(o, s) : o ∈ O, s ∈ S}
SO ⊆ S × O = {(s, o) : s ∈ S, o ∈ O}
RE ⊆ R × E = {(r, e) : r ∈ R, e ∈ E}
ER ⊆ E × R = {(e, r) : e ∈ E, r ∈ R}
```

## 3. Reduction of Triplets to Duplets

### 3.1. Theorem on Representation Equivalence

**Theorem 3.1.** The set RM can be equivalently represented by four sets of duplets:

```
RM = {ER, OS, RE, SO}
```

where:
```
E = SO ⊆ SO × OS = {(so, os) : so ∈ SO, os ∈ OS}
O = ER ⊆ ER × RE = {(er, re) : er ∈ ER, re ∈ RE}
R = OS ⊆ OS × SO = {(os, so) : os ∈ OS, so ∈ SO}
S = RE ⊆ RE × ER = {(re, er) : re ∈ RE, er ∈ ER}
```

### 3.2. Proof of Equivalence

Entity tuple is equivalent to subject-object tuple:
```
e = (s, o, r) = ((s, o), r) = (so, r) ≡ so = (so, re)
```

Object tuple is equivalent to entity-relation tuple:
```
o = (e, r, s) = ((e, r), s) = (er, s) ≡ er = (er, so)
```

Relation tuple is equivalent to object-subject tuple:
```
r = (o, s, e) = ((o, s), e) = (os, e) ≡ os = (os, er)
```

Subject tuple is equivalent to relation-entity tuple:
```
s = (r, e, o) = ((r, e), o) = (re, o) ≡ re = (re, os)
```

## 4. Relations Model as Associative Network

### 4.1. Definitions from Links Theory

**Definition 4.1 (Association).** An association is an ordered pair consisting of a reference and a reference vector:

```
A = L × Vₙ
```

**Definition 4.2 (Associative Network of n-dimensional Vectors).** An associative network is a function:

```
anetⁿ : L → Vₙ
```

mapping a reference l from set L to a reference vector of length n.

**Definition 4.3 (Associative Network of Duplets).** An associative network of duplets (2D associative network):

```
anetd : L → L²
```

### 4.2. Relations Model as Three-Dimensional Associative Network

**Theorem 4.1.** The Relations Model can be represented as a three-dimensional associative network:

```
λ : L → L × L × L = L³
```

where each entity with identifier l is mapped to a triplet (s, r, o).

### 4.3. Reduction to Two-Dimensional Associative Network

**Theorem 4.2.** A three-dimensional associative network can be transformed into a two-dimensional associative network (duplet network) without information loss.

The transformation is performed through nested ordered pairs:

```
anetl : L → NP
```

where NP = {∅ | (l, np), l ∈ L, np ∈ NP} — the set of nested ordered pairs.

## 5. Triune Entity

### 5.1. Definition of Triune Entity

**Definition 5.1.** A triune entity is an element of the Relations Model defined by a triplet (s, r, o), where:

- **s (subject)** — subject, defining the view (View)
- **r (relation)** — relation, defining the controller (Controller)
- **o (object)** — object, defining the model (Model)

### 5.2. Entity Topologies

**Definition 5.2 (Topologies).** Depending on the link structure, entities form various topologies:

1. **Static self-value** (EntId = SubId = RelId = ObjId):
   ```
   e(e -e-> e)
   ```
   The entity structure is topologically closed on itself through itself.

2. **Dynamic self-value** (EntId = SubId = ObjId):
   ```
   e(e -r-> e)
   ```
   The entity structure is closed on itself through another relation-entity.

3. **Reference/storage** (EntId = SubId):
   ```
   e(e -r-> o)
   ```
   The entity stores the projection of another entity.

4. **Projection source** (EntId = ObjId):
   ```
   e(s -r-> e)
   ```
   The entity forms its own projection for perception by other entities.

## 6. Properties of the Relations Model

### 6.1. Homoiconicity

The Relations Model possesses the property of homoiconicity: code and data have the same structure (JSON representation).

### 6.2. Reflexivity

The Relations Model supports reflection: a program can analyze and modify its own structure at runtime.

### 6.3. Closure

**Theorem 6.1 (Closure).** The sets E, O, R, S are equivalent. Any entity can act as subject, relation, or object in other entities.

### 6.4. Fractality

The structure of the Relations Model is fractal: each entity contains three other entities, each of which is also triune.

## 7. JSON Representation

### 7.1. Type Correspondence

| JSON Type | Interpretation in RM |
|-----------|---------------------|
| Number    | Topologically closed projection of numeric relation |
| Boolean   | Topologically closed projection of boolean relation |
| String    | Topologically closed projection of string relation or entity name |
| Object    | Projection of containment relation or entity (if $rel exists) |
| Array     | Projection of sequential relation chain |
| Null      | Empty space (potential for projection containment) |

### 7.2. Executable Constructs

An entity in JSON representation is defined by an object with fields:

```json
{
  "$sub": "<subject>",
  "$rel": "<relation>",
  "$obj": "<object>"
}
```

## 8. Analogy with Molecular Biology

An interesting analogy can be drawn with the complementarity principle of nucleotides in DNA:

- Adenine (A) bonds only with thymine (T) — double bond
- Guanine (G) bonds only with cytosine (C) — triple bond

Similarly in the Relations Model:
- OS is conjugate with SO
- ER is conjugate with RE

This property of preserving relative association type ensures structural integrity.

## 9. Conclusion

The mathematical formalization of the Relations Model in terms of set theory shows that:

1. Triplets (tuples of length 3) can be equivalently represented as nested ordered pairs (two duplets).

2. The Relations Model is a three-dimensional associative network of the form λ : L → L³.

3. The four interconnected sets {E, O, R, S} are equivalent to four sets of duplets {ER, OS, RE, SO}.

4. The triune entity implements the MVC pattern at the level of the basic language structure.

## References

1. Links Theory — https://habr.com/ru/articles/895896/
2. Deep Theory — https://github.com/deep-foundation/deep-theory
3. Associative Model of Data — https://en.wikipedia.org/wiki/Associative_model_of_data
4. Binary Relations — https://en.wikipedia.org/wiki/Binary_relation
