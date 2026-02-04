# Makefile for Relations Model Rocq/Coq Proofs
#
# This Makefile compiles the mathematical formalization of the
# Relations Model (Модель Отношений) from jsonRVM.
#
# Usage:
#   make          - Build all proofs
#   make clean    - Remove generated files
#   make check    - Verify proofs compile correctly

COQC = coqc
COQDEP = coqdep
COQFLAGS = -Q . RelationsModel

VFILES = RelationsModel.v
VOFILES = $(VFILES:.v=.vo)
GLOBFILES = $(VFILES:.v=.glob)

.PHONY: all clean check

all: $(VOFILES)

%.vo: %.v
	$(COQC) $(COQFLAGS) $<

depend: $(VFILES)
	$(COQDEP) $(COQFLAGS) $(VFILES) > .depend

clean:
	rm -f $(VOFILES) $(GLOBFILES) .depend *.vok *.vos .*.aux

check: all
	@echo "All proofs compiled successfully!"

-include .depend
