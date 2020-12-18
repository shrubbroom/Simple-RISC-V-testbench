#!/bin/bash
set -e
TESTBENCH=riscv_soc_tb
srcs=(single pipeline)
project_root=$(pwd)
rm -rf testbench
mkdir testbench
cd testbench
mkdir datalog
for src in ${srcs[*]}; do
    for i in {1..30}; do echo -n "-"; done
    echo
    echo -n "Testing "
    echo -n $src
    echo
    rm -rf run
    mkdir run
    cp "$project_root/assembler/assembler.c" run/
    cp ../$src/*.v run/
    cp "$project_root/emulator/emulator.cpp" run/
    cp ../test/*.asm run/
    cd run
    echo "Begin test"
    echo "Compiling emulator..."
    g++ -o emulator ./emulator.cpp
    echo "Compiling assembler..."
    gcc -o assembler ./assembler.c
    for a in *.asm; do
        for i in {1..20}; do echo -n "-"; done
        echo
        echo -n "Testing "
        echo -n $src " " $a
        echo
        rm -f data_mem_emu.txt data_mem_verilog.txt register_file_emu.txt register_file_verilog.txt test_data_mem.txt
        mv $a assembly.asm
        echo "Compiling ASM..."
        echo | ./assembler > /dev/null
        mv assembly.asm $a
        for i in {1..1024}; do echo "00" >> test_data_mem.txt; done
        cp -f test_data_mem.txt data_mem_emu.txt
        cp -f test_data_mem.txt data_mem.txt
        echo "Compiling Verilog src..."
        iverilog -o $TESTBENCH.vvp $TESTBENCH.v
        echo "Begin Verilog simulation..."
        vvp $TESTBENCH.vvp > vvp.log
        echo "Begin emulator..."
        ./emulator x > /dev/null
        echo "Comparing Verilog result with emulator's result..."
        diff data_mem_emu.txt data_mem_verilog.txt
        diff register_file_emu.txt register_file_verilog.txt
        echo "Creating log: ${src}.${a}"
        mkdir "${project_root}/testbench/datalog/${src}${a}"
        mv "${project_root}/testbench/run/data_mem_emu.txt" "${project_root}/testbench/datalog/${src}${a}/"
        mv "${project_root}/testbench/run/data_mem_verilog.txt" "${project_root}/testbench/datalog/${src}${a}/"
        mv "${project_root}/testbench/run/register_file_emu.txt" "${project_root}/testbench/datalog/${src}${a}/"
        mv "${project_root}/testbench/run/register_file_verilog.txt" "${project_root}/testbench/datalog/${src}${a}/"
        echo -n "Testing "
        echo -n $a
        echo -n " done"
        echo
        for i in {1..20}; do echo -n "-"; done
        echo
        echo
    done
    cd ../
    echo -n "Testing "
    echo -n $src
    echo -n " Done"
    echo
    for i in {1..30}; do echo -n "-"; done
    echo
    echo
    echo
done
