run:
	gcc manipula.c -o manipula
	./manipula

run-test:
	rm -f dados
	rm -f arvore
	gcc manipula.c -o manipula
	./manipula < ./in/1.in > 1-p.out
	cmp ./out/1.out 1-p.out
	rm 1-p.out