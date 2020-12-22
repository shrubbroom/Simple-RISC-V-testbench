#!/usr/bin/pwsh
$ErrorActionPreference = "Stop"
$TESTBENCH = @("riscv_soc_tb")
$srcs = @("single", "pipeline")
$project_root = @($(pwd))
rm -r -fo testbench -ErrorAction Ignore
new-item testbench -itemtype directory
cd testbench
new-item datalog -itemtype directory
for ($k=0; $k -lt $srcs.length; $k++)
{
    $argu = @($srcs[$k])
	for ($i=0; $i -lt 30; $i++)
	{
		write-host "-" -NoNewLine
	}
	write-host
	write-host "Testing " -NoNewLine
	write-host $argu -NoNewLine
	write-host
	rm -r -fo run -ErrorAction Ignore
	new-item run -itemtype directory
	cp -Path "$project_root\assembler\assembler.exe" -Destination .\run
    cp -Path ..\$argu\*.v -Destination .\run
    cp -Path "$project_root\emulator\emulator.exe" -Destination .\run
    cp -Path ..\test\*.asm -Destination .\run
    cd run
    write-host "Begin test"
	$asmfiles = Get-ChildItem ".\*.asm"
    foreach ($j in $asmfiles)
	{
        $a = $j.name
		for ($i=0; $i -lt 20; $i++)
		{
		write-host "-" -NoNewLine
		}
		write-host
		write-host "Testing $argu $a"
		write-host
		rm -fo data_mem_emu.txt, data_mem_verilog.txt, register_file_emu.txt, register_file_verilog.txt, test_data_mem.txt -ErrorAction Ignore
		mv $a assembly.asm
		write-host "Compiling ASM..."
		start-process ./assembler.exe  -windowstyle Hidden -Wait
        # Wait-Process -Name "assembler"
		mv assembly.asm $a
		for ($m=0; $m -lt 1024; $m++)
		{
		write-output "00" | add-content test_data_mem.txt
		}
		cp test_data_mem.txt data_mem_emu.txt
        cp test_data_mem.txt data_mem.txt
        write-host "Compiling Verilog src..."
        start-process iverilog -ArgumentList "-o", "$TESTBENCH.vvp", "$TESTBENCH.v" -WindowStyle Hidden -Wait
        # Wait-Process -Name "iverilog"
        write-host "Begin Verilog simulation..."
        # start-process vvp -WindowStyle Hidden -args "$TESTBENCH.vvp" | out-file vvp.log
        start-process vvp -args "$TESTBENCH.vvp" -WindowStyle Hidden -Wait
        # Wait-Process -Name "vvp"
        # vsim -c do "run -all" "$TESTBENCH.v"
        write-host "Begin emulator..."
        start-process ./emulator.exe -windowStyle Hidden -args "x" -Wait
        # Wait-Process -Name "emulator"
        write-host "Comparing Verilog result with emulator's result..."
		compare-object (get-content data_mem_emu.txt) (get-content data_mem_verilog.txt)
        compare-object (get-content register_file_emu.txt) (get-content register_file_verilog.txt)
        write-host "Creating log: $argu . $a"
        new-item "${project_root}\testbench\datalog\${src}${a}" -itemtype directory
        mv "${project_root}\testbench\run\data_mem_emu.txt" "${project_root}\testbench\datalog\$argu\$a\"
        mv "${project_root}\testbench\run\data_mem_verilog.txt" "${project_root}\testbench\datalog\$argu\$a\"
        mv "${project_root}\testbench\run\register_file_emu.txt" "${project_root}\testbench\datalog\$argu\$a\"
        mv "${project_root}\testbench\run\register_file_verilog.txt" "${project_root}\testbench\datalog\$argu\$a\"
        write-host "Testing  $a  done"
		write-host
        for ($i=0; $i -lt 20; $i++)
		{
		write-host "-" -NoNewLine
		}
        writre-host
    }
    cd ../
    write-host "Testing $argu done"
    write-host
    for ($i=0; $i -lt 20; $i++)
	{
	write-host "-" -NoNewLine
	}
}

	
