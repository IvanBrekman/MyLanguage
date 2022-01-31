cr:
	clear
	gcc main.cpp   libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp libs/tree.cpp 										\
	               src/Frontend/SyntaxAnalyzer/anal.cpp src/Frontend/Tokenizer/token.cpp src/Frontend/front.cpp				\
				   src/Backend/back.cpp src/Backend/AssemblerGenerator/asm_gen.cpp	\
				   src/StandardLibrary/functions.cpp																		\
		-lstdc++ -lm -I . -o main.out
	./main.out test.txt

c:
	gcc main.cpp   libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp libs/tree.cpp 										\
	               src/Frontend/SyntaxAnalyzer/anal.cpp src/Frontend/Tokenizer/token.cpp src/Frontend/front.cpp				\
				   src/Backend/back.cpp src/Backend/AssemblerGenerator/asm_gen.cpp	\
				   src/StandardLibrary/functions.cpp																		\
		-lstdc++ -lm -I . -o main.out

r:
	./main.out

pc:
	gcc   libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp				\
		  src/Backend/Processor/main.cpp src/Backend/Processor/run_cpu.cpp	\
		-lm -I . -o logs/processor_work/processor.out
