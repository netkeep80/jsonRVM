cd doc
dot -O -Tjpg array.dot
dot -O -Tjpg tree_stack_of_contexts.dot
cd ..
rmdir /S /Q docs\html
doxygen Doxyfile