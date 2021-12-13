//
// Created by ivanbrekman on 15.10.2021.
//

#ifndef SIMPLEPROCESSOR_CPU_H
#define SIMPLEPROCESSOR_CPU_H

//! Function execute commands in executable_file
//! \param executable_file pointer to executable file
//! \return                exit code of executing program
int execute(const char* executable_file);

//! Function execute bin commands
//! \param mcodes     pointer to object of BinCommand structure with commands
//! \param n_commands amount of bin commands
//! \param processor  pointer to object of Processor structure
//! \return           exit code of program
//! \note this function called automatically in execute function
int execute_commands(BinCommand* mcodes, int n_commands, Processor* processor);

#endif //SIMPLEPROCESSOR_CPU_H
