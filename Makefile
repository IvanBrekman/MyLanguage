cr:
	clear
	gcc main.cpp   libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp libs/tree.cpp 										\
	               src/Frontend/SyntaxAnalizator/analizator.cpp src/Frontend/Tokenizator/token.cpp src/Frontend/front.cpp	\
				   src/Backend/back.cpp																						\
		-lstdc++ -lm -o main.out
	./main.out

c:
	gcc main.cpp   libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp libs/tree.cpp 										\
	               src/Frontend/SyntaxAnalizator/analizator.cpp src/Frontend/Tokenizator/token.cpp src/Frontend/front.cpp	\
				   src/Backend/back.cpp																						\
		-lstdc++ -lm -o main.out

r:
	./main.out

pc:
	gcc   libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp				\
		  src/Backend/Processor/main.cpp src/Backend/Processor/run_cpu.cpp	\
		-lm -o src/Backend/Processor/processor.out
