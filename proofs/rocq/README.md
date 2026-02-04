# Relations Model - Rocq/Coq Proofs

This directory contains the formal mathematical specification and proofs for the Relations Model (Модель Отношений) in the Rocq proof assistant (formerly known as Coq).

## Files

- `RelationsModel.v` - Main formalization containing:
  - Basic definitions (references, duplets, triplets)
  - Triune entity structure
  - Entity topologies
  - Link type classification
  - Associative network representations
  - Conversion theorems between 3D and 2D networks
  - Model properties (closure, homoiconicity, fractality)

## Requirements

- Rocq/Coq 8.16+ (or Coq 8.13+)
- Standard library only (no external dependencies)

## Building

```bash
# Using make
make

# Or directly with coqc
coqc -Q . RelationsModel RelationsModel.v
```

## Verification

The formalization proves the following key theorems:

1. **Triplet-Duplet Equivalence (Theorem 3.1)**
   - Triplets can be represented as nested ordered pairs without information loss

2. **3D to 2D Network Transformation (Theorem 4.2)**
   - 3D associative networks can be converted to 2D duplet networks

3. **Conjugation Involutivity**
   - Link type conjugation is involutive (applying twice returns original)

## Structure

The formalization follows the mathematical description in:
- `doc/relations-model-ru.md` (Russian)
- `doc/relations-model-en.md` (English)

## References

- [Links Theory (Теория связей)](https://habr.com/ru/articles/895896/)
- [Deep Theory](https://github.com/deep-foundation/deep-theory)
- [Associative Model of Data](https://en.wikipedia.org/wiki/Associative_model_of_data)
