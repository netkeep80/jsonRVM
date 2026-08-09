# Portfolio roadmap

`jsonRVM` является частью portfolio [`netkeep80`](https://github.com/netkeep80).

Portfolio-level направление, lifecycle и migration role **намеренно не дублируются здесь**. Authoritative sources:

- [netkeep80/roadmap](https://github.com/netkeep80/roadmap) — главный portfolio control plane;
- [Current status](https://github.com/netkeep80/roadmap/blob/main/STATUS.md) — live GitHub state;
- [Execution order](https://github.com/netkeep80/roadmap/blob/main/EXECUTION.md) — cross-repo gates;
- [Architecture](https://github.com/netkeep80/roadmap/blob/main/ARCHITECTURE.md) — canonical ownership/dependencies.

Historical runtime code/corpus остаются local source of truth. Решение о роли oracle/migration и downstream AVM ordering хранится централизованно.

```text
roadmap decides portfolio direction;
this repository preserves its local runtime/oracle material;
GitHub facts feed the central live status.
```
