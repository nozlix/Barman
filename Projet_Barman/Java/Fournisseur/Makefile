JC = javac
JFLAGS = -g

.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

default: classes rmic_fournisseur

classes:
	$(JC) $(JFLAGS) *.java

clean:
	rm -rf *.class

.PHONY: clean

rmic_fournisseur:
	rmic Fournisseur