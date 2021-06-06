#!/bin/bash
#script to launch set of tests

#var
origin=/home/practica/eclipse-workspace/c/Debug/c
slave=192.168.2.115
#

echo "testing"

for NP in 5 
do
	for PRUNE in 0 1
	do
		for SEARCH in 1
		do
			for TUPLE in 0 1
			do
				for RR in 0 1
				do
					for RRALL in 0 1
					do
						for ALG in 0 1 2
						do
							for SIZE in 5
							do
								mpirun -np $NP -hosts master,$slave,slave1,slave2 $origin test $SIZE $PRUNE $RR $TUPLE $SEARCH $ALG $RRALL &
								wait $!
								echo "End np $NP data $SIZE prune $PRUNE distrib $RR tuple $TUPLE bd $SEARCH alg $ALG rr_all $RRALL"
							done
						done
					done
				done
			done
		done
	done
done
exit 0
