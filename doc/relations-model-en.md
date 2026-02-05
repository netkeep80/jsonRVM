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
D = L² = L × L = {(b, e) : b ∈ L, e ∈ L}
```

where:
- b — begin (start)
- e — end (finish)

**Definition 1.4 (Triplet).** A triplet (tuple of length 3) is an element of the set:

```
T = L³ = L × L × L = {(r, s, o) : r ∈ L, s ∈ L, o ∈ L}
```

or equivalently:

```
T = L × D, where D = L × L
```

where:
- r — reference to relation
- s — reference to subject
- o — reference to object

### 1.3. Representation of Triplet as Nested Ordered Pairs

From the perspective of set theory, a triplet can be represented as nested ordered pairs:

```
(r, s, o) = (r, (s, o)) = (e, d)
```

where:
- e — reference to entity (in the role of relation)
- d = (s, o) — duplet (subject-object)

This corresponds to the representation of a tuple through a Kuratowski pair.

## 2. Internal Structure of Links

### 2.1. Two Aspects of an Entity

In the Relations Model, each entity can be defined through links:

```
E = ER    (entity is defined by entity-relation pair)
R = SO    (relation is defined by subject-object pair)
S ⊆ E     (subjects are a subset of entities)
O ⊆ E     (objects are a subset of entities)
```

### 2.2. Link Types (Duplets)

We define two basic link types:

```
SO ⊆ S × O = {(s, o) : s ∈ E, o ∈ E}
ER ⊆ E × R = {(e, r) : e ∈ E, r ∈ E}
```

where:
- SO — subject-object duplet, defining a relation
- ER — entity-relation duplet, defining an entity

## 3. Reduction of Triplets to Duplets

### 3.1. Theorem on Representation Equivalence

**Theorem 3.1.** The set RM can be equivalently represented by two subsets of duplets:

```
RM = {ER, SO}
```

where:
```
E = ER ⊆ E × R = {(e, r) : e ∈ E, r ∈ E}
R = SO ⊆ S × O = {(s, o) : s ∈ E, o ∈ E}
```

### 3.2. Proof of Equivalence

A triplet is equivalent to a combination of two duplets:

```
t = (r, s, o) = (r, (s, o)) = (e, d)
```

where:
- e ∈ E — entity in the role of relation
- d = (s, o) ∈ SO — subject-object duplet

Thus, each triplet is uniquely determined by:
1. A reference to the entity-relation (e)
2. A subject-object duplet (d = (s, o))

## 4. Relations Model as a Function

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

### 4.2. Relations Model as a Function

**Definition 4.4 (Relations Model).** The Relations Model can be represented as a function:

```
rm : E → E × E × E = E³
```

where each entity with identifier e is mapped to a triplet (r, s, o).

Equivalently, considering the nested pair structure:

```
rm : E → E × (E × E)
rm(e) = (r, (s, o))
```

### 4.3. Reduction to Two-Dimensional Associative Network

**Theorem 4.1.** A three-dimensional associative network can be transformed into a two-dimensional associative network (duplet network) without information loss.

The transformation is performed through nested ordered pairs:

```
anetl : L → NP
```

where NP = {∅ | (l, np), l ∈ L, np ∈ NP} — the set of nested ordered pairs.

## 5. Triune Entity

### 5.1. Definition of Triune Entity

**Definition 5.1.** A triune entity is an element of the Relations Model defined by a triplet (r, s, o), where:

- **r (relation)** — relation, defining the controller (Controller)
- **s (subject)** — subject, defining the view (View)
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

**Theorem 6.1 (Closure).** The sets E, S, O are equivalent. Any entity can act as subject, relation, or object in other entities.

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
  "$rel": "<relation>",
  "$sub": "<subject>",
  "$obj": "<object>"
}
```

## 8. Conclusion

The mathematical formalization of the Relations Model in terms of set theory shows that:

1. Triplets (tuples of length 3) can be equivalently represented as nested ordered pairs: (r, s, o) = (r, (s, o)) = (e, d).

2. The Relations Model is a function of the form rm : E → E³.

3. Two basic duplet types {ER, SO} are sufficient to represent the entire model.

4. The triune entity implements the MVC pattern at the level of the basic language structure.

## References

1. Links Theory — https://habr.com/ru/articles/895896/
2. Deep Theory — https://github.com/deep-foundation/deep-theory
3. Associative Model of Data — https://en.wikipedia.org/wiki/Associative_model_of_data
4. Binary Relations — https://en.wikipedia.org/wiki/Binary_relation
