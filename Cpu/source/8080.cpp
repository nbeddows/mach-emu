module;
#include <assert.h>
#include <chrono>
#include <coroutine>

module _8080;

using namespace std::chrono;

namespace Emulator
{

Intel8080::Intel8080(const SystemBus<uint16_t, uint8_t, 8>& systemBus)
	: addressBus_(systemBus.addressBus),
	dataBus_(systemBus.dataBus),
	controlBus_(systemBus.controlBus)
{
	int err = fopen_s (&fout_, "cpu_dump.txt", "w");
	
	opcodeTable_ = std::unique_ptr<std::function <CoObj()>[]>(new std::function <CoObj()>[256]
	{
		[&] { return Nop(); },
		[&] { return Lxi(b_, c_); },
		[&] { return Stax(b_, c_); },
		[&] { return Inx(b_, c_); },
		[&] { return Inr(b_); },
		[&] { return Dcr(b_); },
		[&] { return Mvi(b_); },
		[&] { return Rlc(); },
		[&] { return NotImplemented(); },
		[&] { return Dad(b_, c_); },
		[&] { return Ldax(b_, c_); },
		[&] { return Dcx(b_, c_); },
		[&] { return Inr(c_); },
		[&] { return Dcr(c_); },
		[&] { return Mvi(c_); },
		[&] { return Rrc(); },
		[&] { return NotImplemented(); },
		[&] { return Lxi(d_, e_); },
		[&] { return Stax(d_, e_); },
		[&] { return Inx(d_, e_); },
		[&] { return Inr(d_); },
		[&] { return Dcr(d_); },
		[&] { return Mvi(d_); },
		[&] { return Ral(); },
		[&] { return NotImplemented(); },
		[&] { return Dad(d_, e_); },
		[&] { return Ldax(d_, e_); },
		[&] { return Dcx(d_, e_); },
		[&] { return Inr(e_); },
		[&] { return Dcr(e_); },
		[&] { return Mvi(e_); },
		[&] { return Rar(); },
		[&] { return NotImplemented(); },
		[&] { return Lxi(h_, l_); },
		[&] { return Shld(); },
		[&] { return Inx(h_, l_); },
		[&] { return Inr(h_); },
		[&] { return Dcr(h_); },
		[&] { return Mvi(h_); },
		[&] { return Daa(); },
		[&] { return NotImplemented(); },
		[&] { return Dad(h_, l_); },
		[&] { return Lhld(); },
		[&] { return Dcx(h_, l_); },
		[&] { return Inr(l_); },
		[&] { return Dcr(l_); },
		[&] { return Mvi(l_); },
		[&] { return Cma(); },
		[&] { return NotImplemented(); },
		[&] { return Lxi(); },
		[&] { return Sta(); },
		[&] { return Inx(); },
		[&] { return Inr(); },
		[&] { return Dcr(Uint16(h_, l_)); },
		[&] { return Mvi(); },
		[&] { return Stc(); },
		[&] { return NotImplemented(); },
		[&] { return Dad(); },
		[&] { return Lda(); },
		[&] { return Dcx(); },
		[&] { return Inr(a_); },
		[&] { return Dcr(a_); },
		[&] { return Mvi(a_); },
		[&] { return Cmc(); },
		[&] { return Nop(); },
		[&] { return Mov(b_, c_); },
		[&] { return Mov(b_, d_); },
		[&] { return Mov(b_, e_); },
		[&] { return Mov(b_, h_); },
		[&] { return Mov(b_, l_); },
		[&] { return Mov(b_); },
		[&] { return Mov(b_, a_); },
		[&] { return Mov(c_, b_); },
		[&] { return Nop(); },
		[&] { return Mov(c_, d_); },
		[&] { return Mov(c_, e_); },
		[&] { return Mov(c_, h_); },
		[&] { return Mov(c_, l_); },
		[&] { return Mov(c_); },
		[&] { return Mov(c_, a_); },
		[&] { return Mov(d_, b_); },
		[&] { return Mov(d_, c_); },
		[&] { return Nop(); },
		[&] { return Mov(d_, e_); },
		[&] { return Mov(d_, h_); },
		[&] { return Mov(d_, l_); },
		[&] { return Mov(d_); },
		[&] { return Mov(d_, a_); },
		[&] { return Mov(e_, b_); },
		[&] { return Mov(e_, c_); },
		[&] { return Mov(e_, d_); },
		[&] { return Nop(); },
		[&] { return Mov(e_, h_); },
		[&] { return Mov(e_, l_); },
		[&] { return Mov(e_); },
		[&] { return Mov(e_, a_); },
		[&] { return Mov(h_, b_); },
		[&] { return Mov(h_, c_); },
		[&] { return Mov(h_, d_); },
		[&] { return Mov(h_, e_); },
		[&] { return Nop(); },
		[&] { return Mov(h_, l_); },
		[&] { return Mov(h_); },
		[&] { return Mov(h_, a_); },
		[&] { return Mov(l_, b_); },
		[&] { return Mov(l_, c_); },
		[&] { return Mov(l_, d_); },
		[&] { return Mov(l_, e_); },
		[&] { return Mov(l_, h_); },
		[&] { return Nop(); },
		[&] { return Mov(l_); },
		[&] { return Mov(l_, a_); },
		[&] { return Mov(Value(b_)); },
		[&] { return Mov(Value(c_)); },
		[&] { return Mov(Value(d_)); },
		[&] { return Mov(Value(e_)); },
		[&] { return Mov(Value(h_)); },
		[&] { return Mov(Value(l_)); },
		[&] { return Hlt(); },
		[&] { return Mov(Value(a_)); },
		[&] { return Mov(a_, b_); },
		[&] { return Mov(a_, c_); },
		[&] { return Mov(a_, d_); },
		[&] { return Mov(a_, e_); },
		[&] { return Mov(a_, h_); },
		[&] { return Mov(a_, l_); },
		[&] { return Mov(a_); },
		[&] { return Nop(); },
		[&] { return Add(b_, "ADD"); },
		[&] { return Add(c_, "ADD"); },
		[&] { return Add(d_, "ADD"); },
		[&] { return Add(e_, "ADD"); },
		[&] { return Add(h_, "ADD"); },
		[&] { return Add(l_, "ADD"); },
		[&] { return Add(Uint16(h_, l_), "ADD"); },
		[&] { return Add(a_, "ADD"); },
		[&] { return Adc(b_, "ADC"); },
		[&] { return Adc(c_, "ADC"); },
		[&] { return Adc(d_, "ADC"); },
		[&] { return Adc(e_, "ADC"); },
		[&] { return Adc(h_, "ADC"); },
		[&] { return Adc(l_, "ADC"); },
		[&] { return Adc(Uint16(h_, l_), "ADC"); },
		[&] { return Adc(a_, "ADC"); },
		[&] { return Sub(b_, "SUB"); },
		[&] { return Sub(c_, "SUB"); },
		[&] { return Sub(d_, "SUB"); },
		[&] { return Sub(e_, "SUB"); },
		[&] { return Sub(h_, "SUB"); },
		[&] { return Sub(l_, "SUB"); },
		[&] { return Sub(Uint16(h_, l_), "SUB"); },
		[&] { return Sub(a_, "SUB"); },
		[&] { return Sbb(b_, "SBB"); },
		[&] { return Sbb(c_, "SBB"); },
		[&] { return Sbb(d_, "SBB"); },
		[&] { return Sbb(e_, "SBB"); },
		[&] { return Sbb(h_, "SBB"); },
		[&] { return Sbb(l_, "SBB"); },
		[&] { return Sbb(Uint16(h_, l_), "SBB"); },
		[&] { return Sbb(a_, "SBB"); },
		[&] { return Ana(b_, "ANA"); },
		[&] { return Ana(c_, "ANA"); },
		[&] { return Ana(d_, "ANA"); },
		[&] { return Ana(e_, "ANA"); },
		[&] { return Ana(h_, "ANA"); },
		[&] { return Ana(l_, "ANA"); },
		[&] { return Ana(Uint16(h_, l_), "ANA"); },
		[&] { return Ana(a_, "ANA"); },
		[&] { return Xra(b_, "XRA"); },
		[&] { return Xra(c_, "XRA"); },
		[&] { return Xra(d_, "XRA"); },
		[&] { return Xra(e_, "XRA"); },
		[&] { return Xra(h_, "XRA"); },
		[&] { return Xra(l_, "XRA"); },
		[&] { return Xra(Uint16(h_, l_), "XRA"); },
		[&] { return Xra(a_, "XRA"); },
		[&] { return Ora(b_, "ORA"); },
		[&] { return Ora(c_, "ORA"); },
		[&] { return Ora(d_, "ORA"); },
		[&] { return Ora(e_, "ORA"); },
		[&] { return Ora(h_, "ORA"); },
		[&] { return Ora(l_, "ORA"); },
		[&] { return Ora(Uint16(h_, l_), "ORA"); },
		[&] { return Ora(a_, "ORA"); },
		[&] { return Cmp(b_, "CMP"); },
		[&] { return Cmp(c_, "CMP"); },
		[&] { return Cmp(d_, "CMP"); },
		[&] { return Cmp(e_, "CMP"); },
		[&] { return Cmp(h_, "CMP"); },
		[&] { return Cmp(l_, "CMP"); },
		[&] { return Cmp(Uint16(h_, l_), "CMP"); },
		[&] { return Cmp(a_, "CMP"); },
		[&] { return RetOnFlag(status_.test(Condition::ZeroFlag) == false, "RNZ"); },
		[&] { return Pop(b_, c_); },
		[&] { return JmpOnFlag(status_.test(Condition::ZeroFlag) == false, "JNZ"); },
		[&] { return JmpOnFlag(true, "JMP"); },
		[&] { return CallOnFlag(status_.test(Condition::ZeroFlag) == false, "CNZ"); },
		[&] { return Push(b_, c_); },
		[&] { return Add(++pc_, "ADI"); },
		[&] { return Rst(); },
		[&] { return RetOnFlag(status_.test(Condition::ZeroFlag) == true, "RZ"); },
		[&] { return RetOnFlag(true, "RET"); },
		[&] { return JmpOnFlag(status_.test(Condition::ZeroFlag) == true, "JZ"); },
		[&] { return NotImplemented(); },
		[&] { return CallOnFlag(status_.test(Condition::ZeroFlag) == true, "CZ"); },
		[&] { return CallOnFlag(true, "CALL"); },
		[&] { return Adc(++pc_, "ACI"); },
		[&] { return Rst(); },
		[&] { return RetOnFlag(status_.test(Condition::CarryFlag) == false, "RNC"); },
		[&] { return Pop(d_, e_); },
		[&] { return JmpOnFlag(status_.test(Condition::CarryFlag) == false, "JNC"); },
		[&] { return Out(); },
		[&] { return CallOnFlag(status_.test(Condition::CarryFlag) == false, "CNC"); },
		[&] { return Push(d_, e_); },
		[&] { return Sub(++pc_, "SUI"); },
		[&] { return Rst(); },
		[&] { return RetOnFlag(status_.test(Condition::CarryFlag) == true, "RC"); },
		[&] { return NotImplemented(); },
		[&] { return JmpOnFlag(status_.test(Condition::CarryFlag) == true, "JC"); },
		[&] { return In(); },
		[&] { return CallOnFlag(status_.test(Condition::CarryFlag) == true, "CC"); },
		[&] { return NotImplemented(); },
		[&] { return Sbb(++pc_, "SBI"); },
		[&] { return Rst(); },
		[&] { return RetOnFlag(status_.test(Condition::ParityFlag) == false, "RPO"); },
		[&] { return Pop(h_, l_); },
		[&] { return JmpOnFlag(status_.test(Condition::ParityFlag) == false, "JPO"); },
		[&] { return Xthl(); },
		[&] { return CallOnFlag(status_.test(Condition::ParityFlag) == false, "CPO"); },
		[&] { return Push(h_, l_); },
		[&] { return Ana(++pc_, "ANI"); },
		[&] { return Rst(); },
		[&] { return RetOnFlag(status_.test(Condition::ParityFlag) == true, "RPE"); },
		[&] { return Pchl(); },
		[&] { return JmpOnFlag(status_.test(Condition::ParityFlag) == true, "JPE"); },
		[&] { return Xchg(); },
		[&] { return CallOnFlag(status_.test(Condition::ParityFlag) == true, "CPE"); },
		[&] { return NotImplemented(); },
		[&] { return Xra(++pc_, "XRI"); },
		[&] { return Rst(); },
		[&] { return RetOnFlag(status_.test(Condition::SignFlag) == false, "RP"); },
		[&] { return Pop(a_, status_); },
		[&] { return JmpOnFlag(status_.test(Condition::SignFlag) == false, "JP"); },
		[&] { return Di(); },
		[&] { return CallOnFlag(status_.test(Condition::SignFlag) == false, "CP"); },
		[&] { return Push(a_, status_); },
		[&] { return Ora(++pc_, "ORI"); },
		[&] { return Rst(); },
		[&] { return RetOnFlag(status_.test(Condition::SignFlag) == true, "RM"); },
		[&] { return Sphl(); },
		[&] { return JmpOnFlag(status_.test(Condition::SignFlag) == true, "JM"); },
		[&] { return Ei(); },
		[&] { return CallOnFlag(status_.test(Condition::SignFlag) == true, "CM"); },
		[&] { return NotImplemented(); },
		[&] { return Cmp(++pc_, "CPI"); },
		[&] { return Rst(); },
	});
}

void Intel8080::Dump(bool fileWrite)
{
	if (start_dump == true)
	{
		if (fout_ != nullptr && fileWrite == true)
		{
			fprintf(fout_, "IR: 0x%02x  ", opcode_);
			fprintf(fout_, "PC: 0x%04x  ", pc_);
			fprintf(fout_, "SP: 0x%04x  ", sp_);
			fprintf(fout_, "BC: 0x%02x%02x  ", B(), C());
			fprintf(fout_, "DE: 0x%02x%02x  ", D(), E());
			fprintf(fout_, "HL: 0x%02x%02x  ", H(), L());
			fprintf(fout_, "AF: 0x%02x%02x  ", A(), Value(status_));
			fprintf(fout_, "IC: %I64d\n", totalTp_);
		}
		else
		{
			printf("IR: 0x%02x  ", opcode_);
			printf("PC: 0x%04x  ", pc_);
			printf("SP: 0x%04x  ", sp_);
			printf("BC: 0x%02x%02x  ", B(), C());
			printf("DE: 0x%02x%02x  ", D(), E());
			printf("HL: 0x%02x%02x  ", H(), L());
			printf("AF: 0x%02x%02x  ", A(), Value(status_));
			printf("IC: %I64d\n", totalTp_);
		}
	}
}

CoObj Intel8080::Fetch()
{
	//The instruction is not complete (0)
	//and we don't know the time periods
	//until the instruction is fetched, so we
	//use a negative (invalid) value to
	//indicate this.
	timePeriods_ = -1;

	//Fetch the next instruction
	ReadFromAddress(Signal::MemoryRead, pc_);
	co_await coObj_;
	opcode_ = dataBus_->Receive();
	co_return;
}

//bool Intel8080::Execute()
uint8_t Intel8080::Execute()
{
	//Acknowledge the interrupt
	if (controlBus_->Receive(Signal::Interrupt) == true)
	{
		//Fetch the interrupt service routine
		auto isr = dataBus_->Receive();
	
		if (iff_ == true)
		{
			isr_ = static_cast<ISR>(isr);
			
			//the interrupt enable system is automatically
			//disabled whenever an interrupt is acknowledged
			iff_ = false;
		}
	}

	if (coDone_ == true)
	{
		//We want to start a new instruction but the required number
		//of time periods have not elapsed.
		if (timePeriods_ == 0)
		{
			//We didn't complete the instruction in time, this should never happen
			assert (!coObj_.coh || coObj_.coh.done() == true);
			
			//Execute the next instruction if there are no outstanding interrupts
			if (isr_ == ISR::NoInterrupt)
			{
				coObj_ = Fetch();
			}
			else
			{
				opcode_ = 0xC7 | (static_cast<uint8_t>(isr_) << 3);
				coObj_ = Rst(opcode_);

				//coObj_ = Rst(0xC7 | (static_cast<uint8_t>(isr_) << 3));
			
				//Interrupt is being serviced, clear it.
				isr_ = ISR::NoInterrupt;
			}


			//if (sp_ < 0x23DE)
			//{
			//	printf ("STACK CORRUPTION: %d\n", sp_);
			//}

			//if (pc_ == 0x8)
			//	printf("RST 1\n");
			//else if (pc_ == 0x10)
			//	printf("RST 2\n");
			//else if (pc_ == 0x87)
			//	printf("Leaving RST\n");
		}
		//Decode and execute the instruction after we complete the fetch.
		else if (timePeriods_ == -1)
		{
			coObj_ = opcodeTable_[opcode_]();
		}
/*
		else
		{
			//We are essentially spinning here on instruction completion.
		}
*/
	}
	else
	{
		//Resume execution of the current instruction.
		coObj_.coh.resume();
	}

	coDone_ = coObj_.coh.done();

	uint8_t tp = 0;

	if (coDone_ == true && timePeriods_ != -1)
	{
		if ((opcode_ & 0xC7) != 0xC7)
		{
			tp = timePeriods_;
			totalTp_ += tp;
		}

		timePeriods_ = 0;

		//if (pc_ == 0x09EE)
		{
			start_dump = true;
		}

		//Dump(true);

		//if (pc_ == 0x1857)
		if (totalTp_ == 21457942)
		{
			pc_ = pc_;
		}

	}

	return tp;
	//else
	//{
	//	return 0;
	//}
	//if (timePeriods_ > 0)
	//{
	//	timePeriods_--;
	//}

	//return timePeriods_ == 0;
}

//This essentially powers on the cpu
void Intel8080::Reset(uint16_t pc)
{
	a_.reset();
	b_.reset();
	c_.reset();
	d_.reset();
	e_.reset();
	h_.reset();
	l_.reset();
	pc_ = pc;
	sp_ = 0;
	status_ = 0b00000010;
	timePeriods_ = 0;
	iff_ = false;
}

void Intel8080::ReadFromAddress(Signal readLocation, uint16_t addr)
{
	controlBus_->Send(readLocation);			
	addressBus_->Send (addr);
}

void Intel8080::WriteToAddress(Signal writeLocation, uint16_t addr, uint8_t value)
{
	controlBus_->Send(writeLocation);
	addressBus_->Send(addr);
	dataBus_->Send(value);
}

/**
	INR

	The specified register or memory byte is incremented by one.
*/
CoObj Intel8080::Inr(Register& r)
{
	timePeriods_ = 5;
	r = Add(r, 0x01, false, 0, "INR");
	co_return;
}

CoObj Intel8080::Inr()
{
	//Update the emulated time to complete.
	timePeriods_ = 10;
	auto addr = Uint16(h_, l_);
	
	ReadFromAddress(Signal::MemoryRead, addr);
	//Wait for the read request to be handled.
	co_await coObj_;
	
	//Get the data and process it.
	Register r = dataBus_->Receive();
	r = Add(r, 0x01, false, 0, "INR");
	//Inr(r);
	
	WriteToAddress(Signal::MemoryWrite, addr, Value(r));
	//Wait for the write request to be handled.
	co_await coObj_;
	co_return;
}

/**
	DCR

	The specified register or memory byte is
	decremented by one.
*/
CoObj Intel8080::Dcr(Register& r)
{
	timePeriods_ = 5;
	//Using twos compliment add for subtraciton.
	r = Add(r, 0xFF, false, 0, "DCR");
	co_return;
}

CoObj Intel8080::Dcr(uint16_t addr)
{
	timePeriods_ = 10;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	Register r = dataBus_->Receive();
	r = Add(r, 0xFF, false, 0, "DCR");
	//Dcr(r);
	WriteToAddress (Signal::MemoryWrite, addr, Value(r));
	co_await coObj_;
	co_return;
}

CoObj Intel8080::Mvi(Register& reg)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	reg = dataBus_->Receive();

	if constexpr (dbg == true)
	{
		printf("0x%04X MVI %c, 0x%02X\n", pc_ - 1, registerName_[(opcode_ & 0x38) >> 3], Value(reg));
	}

	++pc_;
	co_return;
}

CoObj Intel8080::Mvi()
{
	timePeriods_ = 10;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto data = dataBus_->Receive();
	auto addr = Uint16(h_, l_);

	if constexpr (dbg == true)
	{
		printf("0x%04X MVI [0x%04X], 0x%02X\n", pc_ - 1, addr, data);
	}

	WriteToAddress (Signal::MemoryWrite, addr, data);
	co_await coObj_;

	++pc_;
	co_return;
}

/**
	DAA: Decimal Adjust Accumulator

	The eight-bit hexadecimal number in the
	accumulator is adjusted to form two four bit
	binary-coded-decimal digits.
*/
CoObj Intel8080::Daa()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X DAA\n", pc_);
	}

	uint8_t nibble = Value(a_) & 0x0F;

	if (nibble > 0x09 || status_[Condition::AuxCarryFlag] == true)
	{
		status_[Condition::AuxCarryFlag] = nibble + 0x06 > 0x0F;

		a_ = Value(a_) + 6;
	}

	nibble = (Value(a_) & 0xF0) >> 4;

	if (nibble > 0x09 || status_[Condition::CarryFlag] == true)
	{
		if (nibble + 0x06 > 0x0F)
		{
			status_[Condition::CarryFlag] = true;
		}

		nibble += 6;

		a_ = (nibble << 4) | (Value(a_) & 0x0F);
	}

	status_[Condition::ZeroFlag] = Zero(a_);
	status_[Condition::SignFlag] = Sign(a_);
	status_[Condition::ParityFlag] = Parity(a_);
	++pc_;
	timePeriods_ = 4;
	co_return;
}

/**
	RLC

	The Carry bit is set equal to the high order bit of the accumulator.
	The contents of the accumulator are rotated one bit position to the left
	with the high order bit being transferred to the low-order bit position of
	the accumulator.
*/
CoObj Intel8080::Rlc()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X RLC\n", pc_);
	}

	status_[CarryFlag] = a_[7];
	a_ <<= 1;
	a_[0] = status_[CarryFlag];
	++pc_;
	timePeriods_ = 4;
	co_return;
}

/**
	RRC

	The carry bit is set equal to the low-order
	bit of the accumulator. The contents of the accumulator are
	rotated one bit position to the right, with the low-order bit
	being transferred to the high-order bit position of the accumulator.
*/
CoObj Intel8080::Rrc()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X RRC\n", pc_);
	}

	status_[CarryFlag] = a_[0];
	a_ >>= 1;
	a_[7] = status_[CarryFlag];
	++pc_;
	timePeriods_ = 4;
	co_return;
}

/**
	RAL: Rotate Accumulator Left Through Carry

	The contents of the accumulator are rotated one bit position to the left.
	The high-order bit of the accumulator replaces the
	Carry bit, while the Carry bit replaces the high-order bit of
	the accumulator.
*/
CoObj Intel8080::Ral()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X RAL\n", pc_);
	}

	bool tmp = status_[CarryFlag];
	status_[CarryFlag] = a_[7];
	a_ <<= 1;
	a_[0] = tmp;
	++pc_;
	timePeriods_ = 4;
	co_return;
}

/**
	RAR: Rotate Accumulator Right Through Carry

	The contents of the accumulator are rotated one bit position to the right.
	The low-order bit of the accumulator replaces the
	carry bit, while the carry bit replaces the high-order bit of
	the accumulator.
*/
CoObj Intel8080::Rar()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X RAR\n", pc_);
	}
	
	bool tmp = status_[CarryFlag];
	status_[CarryFlag] = a_[0];
	a_ >>= 1;
	a_[7] = tmp;
	++pc_;
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Lxi(Register& regHi, Register& regLow)
{
	timePeriods_ = 10;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	regLow = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	regHi = dataBus_->Receive();

	if constexpr (dbg == true)
	{
		printf("0x%04X LXI %c, 0x%04X\n", pc_ - 2, registerName_[(opcode_ & 0x30) >> 3], Uint16(Value(regHi), Value(regLow)));
	}

	++pc_;
	co_return;
}

CoObj Intel8080::Lxi()
{
	timePeriods_ = 10;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto spLow = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	sp_ = Uint16(dataBus_->Receive(), spLow);

	if constexpr (dbg == true)
	{
		printf("0x%04X LXI SP, 0x%04X\n", pc_ - 2, sp_);
	}

	++pc_;
	co_return;
}

/**
	SHLD: Store H and L Direct

	The contents of the L register are stored
	at the memory address formed by concatenating HI ADD
	with LOW ADO. The contents of the H register are stored at
	the next higher memory address.
*/
CoObj Intel8080::Shld()
{
	timePeriods_ = 16;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto addrLow = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	uint16_t addr = Uint16(dataBus_->Receive(), addrLow);

	if constexpr (dbg == true)
	{
		printf("0x%04X SHLD, [0x%04X]\n", pc_ - 2, addr);
	}

	WriteToAddress(Signal::MemoryWrite, addr, Value(l_));
	co_await coObj_;
	WriteToAddress(Signal::MemoryWrite, addr + 1, Value(h_));
	co_await coObj_;
	++pc_;
	co_return;
}

/**
	STAX

	Description: The contents of the accumulator are
	stored in the memory location addressed by registers B and
	C, or by registers D and E.
	Condition bits affected: None
	Example:
	If register B contains 3FH and register C contains
	16H, the instruction: STAX B
	will store the contents of the accumulator at memory location 3F16H.
*/
CoObj Intel8080::Stax(const Register& hi, const Register& low)
{
	timePeriods_ = 7;

	if constexpr (dbg == true)
	{
		printf("0x%04X STAX %c\n", pc_, registerName_[(opcode_ & 0x10) >> 3]);
	}

	WriteToAddress(Signal::MemoryWrite, Uint16(hi, low), Value(a_));
	co_await coObj_;
	++pc_;
	co_return;
}

/**
	INX

	The 16-bit number held in the specified
	register pair is incremented by one.
*/
CoObj Intel8080::Inx(Register& hi, Register& low)
{
	if constexpr (dbg == true)
	{
		printf("0x%04X INX %c\n", pc_, registerName_[(opcode_ & 0x30) >> 3]);
	}

	uint16_t val = Uint16(hi, low) + 1;
	hi = (val >> 8) & 0xFF;
	low = val & 0xFF;
	++pc_;
	timePeriods_ = 5;
	co_return;
}

CoObj Intel8080::Inx()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X INX SP\n", pc_);
	}

	++sp_;
	++pc_;
	timePeriods_ = 5;
	co_return;
}

/**
	DAD

	The 16-bit number in the BC/DE pair is added to the 16-bit number held in the H and L
	registers using two's complement arithmetic. The result replaces the contents of
	the H and L registers.
*/
CoObj Intel8080::Dad(const Register& hi, const Register& low)
{
	if constexpr (dbg == true)
	{
		if ((opcode_ & 0x30) == 0x30)
		{
			printf("0x%04X DAD SP\n", sp_);
		}
		else
		{
			printf("0x%04X DAD %c\n", pc_, registerName_[(opcode_ & 0x30) >> 3]);
		}
	}

	uint32_t val = Uint16(hi, low) + Uint16(h_, l_);
	h_ = (val >> 8) & 0xFF;
	l_ = val & 0xFF;
	status_[Condition::CarryFlag] = (val > 0xFFFF);
	++pc_;
	timePeriods_ = 10;
	co_return;
}

CoObj Intel8080::Dad()
{
	Dad((sp_ >> 8) & 0xFF, sp_ & 0xFF);
	co_return;
}

/**
	LHLD: load H and L direct

	The byte at the memory address formed
	by concatenating HI ADD with LOW ADD replaces the contents of the L register.
	The byte at the next higher memory address replaces the contents of the H register.
*/
CoObj Intel8080::Lhld()
{
	timePeriods_ = 16;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto addrLow = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	uint16_t addr = Uint16(dataBus_->Receive(), addrLow);

	if constexpr (dbg == true)
	{
		printf("0x%04X LHLD, [0x%04X]\n", pc_ - 2, addr);
	}

	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	l_ = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, addr + 1);
	co_await coObj_;
	h_ = dataBus_->Receive();
	++pc_;
	co_return;
}

/**
	LDAX

	The contents of the memory location
	addressed by registers BC/DE replace the contents of the accumulator.
*/
CoObj Intel8080::Ldax(const Register& hi, const Register& low)
{
	timePeriods_ = 7;

	if constexpr (dbg == true)
	{
		printf("0x%04X LDAX, %c\n", pc_, registerName_[(opcode_ & 0x10) >> 3]);
	}

	ReadFromAddress(Signal::MemoryRead, Uint16(hi, low));
	co_await coObj_;
	a_ = dataBus_->Receive();
	++pc_;
	co_return;
}

/**
	DCX

	The 16-bit number held in the specified
	register pair is decremented by one.
*/
CoObj Intel8080::Dcx(Register& hi, Register& low)
{
	if constexpr (dbg == true)
	{
		printf("0x%04X DCX %c\n", pc_, registerName_[(opcode_ & 0x30) >> 3]);
	}

	uint16_t val = Uint16(hi, low) + 0xFFFF;
	hi = (val >> 8) & 0xFF;
	low = val & 0xFF;
	++pc_;
	timePeriods_ = 5;
	co_return;
}

CoObj Intel8080::Dcx()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X DCX SP\n", pc_);
	}

	sp_ += 0xFFFF;
	++pc_;
	timePeriods_ = 5;
	co_return;
}

/**
	CMA

	Each bit of the contents of the accumulator is complemented (producing the one's complement).
*/
CoObj Intel8080::Cma()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X CMA\n", pc_);
	}

	a_.flip();
	++pc_;
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Sta()
{
	timePeriods_ = 13;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto addrLow = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	uint16_t addr = Uint16(dataBus_->Receive(), addrLow);

	if constexpr (dbg == true)
	{
		printf("0x%04X STA, [0x%04X]\n", pc_ - 2, addr);
	}

	WriteToAddress (Signal::MemoryWrite, addr, Value(a_));
	co_await coObj_;
	++pc_;
	co_return;
}

CoObj Intel8080::Stc()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X STC\n", pc_);
	}

	status_[Condition::CarryFlag] = true;
	++pc_;
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Lda()
{
	timePeriods_ = 13;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto addrLow = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	uint16_t addr = Uint16(dataBus_->Receive(), addrLow);

	if constexpr (dbg == true)
	{
		printf("0x%04X LDA, [0x%04X]\n", pc_ - 2, addr);
	}

	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	a_ = dataBus_->Receive();
	++pc_;
	co_return;
}

CoObj Intel8080::Cmc()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X CMC\n", pc_);
	}

	status_[Condition::CarryFlag] = !status_[Condition::CarryFlag];
	pc_++;
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Mov(Register& lhs, const Register& rhs)
{
	if constexpr (dbg == true)
	{
		printf("0x%04X MOV %c, %c\n", pc_, registerName_[(opcode_ & 0x38) >> 3], registerName_[opcode_ & 0x07]);
	}

	lhs = rhs;
	pc_++;
	timePeriods_ = 5;
	co_return;
}

CoObj Intel8080::Mov(Register& lhs)
{
	auto addr = Uint16(h_, l_);
	timePeriods_ = 7;

	if constexpr (dbg == true)
	{
		printf("0x%04X MOV %c, [0x%04X]\n", pc_, registerName_[(opcode_ & 0x38) >> 3], addr);
	}

	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	lhs = dataBus_->Receive();
	pc_++;
	co_return;
}

CoObj Intel8080::Mov(uint8_t value)
{
	uint16_t addr = Uint16(h_, l_);
	timePeriods_ = 7;

	if constexpr (dbg == true)
	{
		printf("0x%04X MOV [0x%04X], %c\n", pc_, addr, registerName_[opcode_ & 0x07]);
	}

	WriteToAddress(Signal::MemoryWrite, addr, value);
	co_await coObj_;
	pc_++;
	co_return;
}

CoObj Intel8080::Nop()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X NOP\n", pc_);
	}

	pc_++;
	timePeriods_ = 4;
	co_return;
}

/**
	HLT

	The program counter is incremented to
	the address of the next sequential instruction.The CPU then
	enters the STOPPED state and no further activity takes
	place until an interrupt occurs.
*/
/*
	Implementation of the HLT instruction steps the
	Program Counter to the next instruction address and stops
	the computer until an interrupt occurs. The HLT instruction
	should not normally be implemented when a DI instruction
	has been executed. Since the DI instruction causes the computer
	to ignore interrupts, the computer will not operate again
	until the main power switch is turned off and then back on.
*/
CoObj Intel8080::Hlt()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X HLT\n", pc_);
	}

	assert(0);

	pc_++;

	//need to wait here .... co-routine??

	timePeriods_ = 7;
	co_return;
}

Intel8080::Register Intel8080::Add(const Register& lhs, const Register& rhs, bool setCarryFlag, uint8_t carry, [[maybe_unused]] std::string_view instructionName)
{
	if constexpr (dbg == true)
	{
		if (instructionName.data() == "ADI" || instructionName.data() == "ACI" || instructionName.data() == "SUI" || instructionName.data() == "SBI" || instructionName.data() == "CPI")
		{
			uint16_t addr = pc_ + 0xFFFF;
			printf("0x%04X %s 0x%02X\n", addr, instructionName.data(), opcode_);
		}
		else
		{
			printf("0x%04X %s %c\n", pc_, instructionName.data(), opcode_ & 0x80 ? registerName_[opcode_ & 0x07] : registerName_[(opcode_ & 0x38) >> 3]);
		}
	}

	uint8_t valuePlusCarry = Value(rhs) + carry;

	if (setCarryFlag == true)
	{
		status_[Condition::CarryFlag] = Carry(lhs, valuePlusCarry);
	}

	status_[Condition::AuxCarryFlag] = AuxCarry(lhs, valuePlusCarry);

	Register r = Value(lhs) + valuePlusCarry;

	status_[Condition::ZeroFlag] = Zero(r);
	status_[Condition::SignFlag] = Sign(r);
	status_[Condition::ParityFlag] = Parity(r);
	pc_++;
	return r;
}

CoObj Intel8080::Add(const Register& r, std::string_view instructionName)
{
	a_ = Add(a_, r, true, 0, instructionName);
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Add(uint16_t addr, std::string_view instructionName)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	a_ = Add(a_, Register(dataBus_->Receive()), true, 0, instructionName);
	co_return;
}


CoObj Intel8080::Adc(const Register& r, std::string_view instructionName)
{
	a_ = Add(a_, r, true, status_[Condition::CarryFlag], instructionName);
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Adc(uint16_t addr, std::string_view instructionName)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	a_ = Add(a_, Register(dataBus_->Receive()), true, status_[Condition::CarryFlag], instructionName);
	co_return;
}

Intel8080::Register Intel8080::Sub(const Register& r, uint8_t withCarry, std::string_view instructionName)
{
	auto reg = Add(a_, ~(Value(r) + withCarry) + 1, true, 0/*withCarry*/, instructionName);
	status_.flip(Condition::CarryFlag);
	return reg;
}

CoObj Intel8080::Sub(const Register& r, std::string_view instructionName)
{
	a_ = Sub(r, 0, instructionName);
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Sub(uint16_t addr, std::string_view instructionName)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	a_ = Sub(Register(dataBus_->Receive()), 0, instructionName);
	co_return;
}

CoObj Intel8080::Sbb(const Register& r, std::string_view instructionName)
{
	a_ = Sub(r, status_[Condition::CarryFlag], instructionName);
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Sbb(uint16_t addr, std::string_view instructionName)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	a_ = Sub(Register(dataBus_->Receive()), status_[Condition::CarryFlag], instructionName);
	co_return;
}

void Intel8080::Ana(const Register& r)
{
	status_[Condition::AuxCarryFlag] = ((a_ | r) & Register(0x08)) != 0;

	a_ &= r;

	status_[Condition::CarryFlag] = false;
	status_[Condition::ZeroFlag] = Zero(a_);
	status_[Condition::SignFlag] = Sign(a_);
	status_[Condition::ParityFlag] = Parity(a_);
	pc_++;
}

CoObj Intel8080::Ana(const Register& r, std::string_view instructionName)
{
	if constexpr (dbg == true)
	{
		printf("0x%04X %s %c\n", pc_, instructionName.data(), opcode_ & 0x80 ? registerName_[opcode_ & 0x07] : registerName_[(opcode_ & 0x38) >> 3]);
	}

	timePeriods_ = 4;
	Ana (r);

	co_return;
}

CoObj Intel8080::Ana(uint16_t addr, std::string_view instructionName)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	Register r = dataBus_->Receive();

	if constexpr (dbg == true)
	{
		if (instructionName.data() == "ANI")
		{
			printf("0x%04X %s 0x%02X\n", pc_ - 1, instructionName.data(), Value(r));
		}
		else
		{
			printf("0x%04X %s %c\n", pc_, instructionName.data(), opcode_ & 0x80 ? registerName_[opcode_ & 0x07] : registerName_[(opcode_ & 0x38) >> 3]);
		}
	}
	
	Ana(r);
	co_return;
}

void Intel8080::Xra(const Register& r)
{
	a_ ^= r;

	status_[Condition::AuxCarryFlag] = false;
	status_[Condition::CarryFlag] = false;
	status_[Condition::ZeroFlag] = Zero(a_);
	status_[Condition::SignFlag] = Sign(a_);
	status_[Condition::ParityFlag] = Parity(a_);
	pc_++;
}

CoObj Intel8080::Xra(const Register& r, std::string_view instructionName)
{
	if constexpr (dbg == true)
	{
		printf("0x%04X %s %c\n", pc_, instructionName.data(), opcode_ & 0x80 ? registerName_[opcode_ & 0x07] : registerName_[(opcode_ & 0x38) >> 3]);
	}

	timePeriods_ = 4;
	Xra(r);
	//status_[Condition::AuxCarryFlag] = AuxCarry(a_, Value(r));
	co_return;
}

CoObj Intel8080::Xra(uint16_t addr, std::string_view instructionName)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	Register r = dataBus_->Receive();

	if constexpr (dbg == true)
	{
		if (instructionName.data() == "XRI")
		{
			printf("0x%04X %s 0x%02X\n", pc_ - 1, instructionName.data(), Value(r));
		}
		else
		{
			printf("0x%04X %s %c\n", pc_, instructionName.data(), opcode_ & 0x80 ? registerName_[opcode_ & 0x07] : registerName_[(opcode_ & 0x38) >> 3]);
		}
	}
	
	Xra(r);
	co_return;
}

void Intel8080::Ora(const Register& r)
{
	a_ |= r;

	status_[Condition::AuxCarryFlag] = false;
	status_[Condition::CarryFlag] = false;
	status_[Condition::ZeroFlag] = Zero(a_);
	status_[Condition::SignFlag] = Sign(a_);
	status_[Condition::ParityFlag] = Parity(a_);
	pc_++;
}

CoObj Intel8080::Ora(const Register& r, std::string_view instructionName)
{
	if constexpr (dbg == true)
	{
		printf("0x%04X %s %c\n", pc_, instructionName.data(), opcode_ & 0x80 ? registerName_[opcode_ & 0x07] : registerName_[(opcode_ & 0x38) >> 3]);
	}

	timePeriods_ = 4;
	Ora(r);
	co_return;
}

CoObj Intel8080::Ora(uint16_t addr, std::string_view instructionName)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	Register r = dataBus_->Receive();

	if constexpr (dbg == true)
	{
		if (instructionName.data() == "ORI")
		{
			printf("0x%04X %s 0x%02X\n", pc_ - 1, instructionName.data(), Value(r));
		}
		else
		{
			printf("0x%04X %s %c\n", pc_, instructionName.data(), opcode_ & 0x80 ? registerName_[opcode_ & 0x07] : registerName_[(opcode_ & 0x38) >> 3]);
		}
	}

	Ora(r);
	co_return;
}

CoObj Intel8080::Cmp(const Register& r, std::string_view instructionName)
{
	int16_t result = Value(a_) - Value(r);
	status_[Condition::CarryFlag] = result >> 8;
	status_[Condition::AuxCarryFlag] = ~(Value(a_) ^ result ^ Value(r)) & 0x10;

	Register reg = static_cast<int8_t>(result);
	status_[Condition::ZeroFlag] = Zero(reg);
	status_[Condition::SignFlag] = Sign(reg);
	status_[Condition::ParityFlag] = Parity(reg);

	timePeriods_ = 4;
	pc_++;
	co_return;
}

CoObj Intel8080::Cmp(uint16_t addr, std::string_view instructionName)
{
	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, addr);
	co_await coObj_;
	Register r = dataBus_->Receive();

	int16_t result = Value(a_) - Value(r);
	status_[Condition::CarryFlag] = result >> 8;
	status_[Condition::AuxCarryFlag] = ~(Value(a_) ^ result ^ Value(r)) & 0x10;
	r = static_cast<int8_t>(result);
	status_[Condition::ZeroFlag] = Zero(r);
	status_[Condition::SignFlag] = Sign(r);
	status_[Condition::ParityFlag] = Parity(r);
	pc_++;
	co_return;
}

CoObj Intel8080::NotImplemented()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X Instruction %02X not implemented\n", pc_, opcode_);
	}

	if (opcode_ == 0xED || opcode_ == 0xFD || opcode_ == 0xDD)
	{
		pc_++;
	}
	else
	{
		assert(0);
	}

	pc_++;

	timePeriods_ = 0;
	co_return;
}

CoObj Intel8080::RetOnFlag(bool status, std::string_view instructionName)
{
	if constexpr (dbg == true)
	{
		printf("0x%04X %s\n", pc_, instructionName.data());
	}

	pc_++;
	
	if (status == true)
	{
		timePeriods_ = 11;

		ReadFromAddress(Signal::MemoryRead, sp_++);
		co_await coObj_;
		auto pcLow = dataBus_->Receive();
		ReadFromAddress(Signal::MemoryRead, sp_++);
		co_await coObj_;
		pc_ = Uint16(dataBus_->Receive(), pcLow);

		if (instructionName == "RET")
		{
			timePeriods_ = 10;
		}
		else
		{
			timePeriods_ = 11;
		}
	}
	else
	{
		timePeriods_ = 5;
	}
	
	co_return;
}

CoObj Intel8080::Pop(Register& hi, Register& low)
{
	if constexpr (dbg == true)
	{
		if ((opcode_ & 0x30) == 0x30)
		{
			printf("0x%04X POP PSW\n", pc_);
		}
		else
		{
			printf("0x%04X POP %c\n", pc_, registerName_[(opcode_ & 0x30) >> 3]);
		}
	}

	timePeriods_ = 10;
	ReadFromAddress(Signal::MemoryRead, sp_++);
	co_await coObj_;
	low = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, sp_++);
	co_await coObj_;
	hi = dataBus_->Receive();
	pc_++;
	co_return;
}

CoObj Intel8080::JmpOnFlag(bool status, std::string_view instructionName)
{
	timePeriods_ = 10;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto addrLow = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto addr = Uint16(dataBus_->Receive(), addrLow);

	if constexpr (dbg == true)
	{
		printf("0x%04X %s 0x%04X\n", pc_ - 2, instructionName.data(), addr);
	}

	status ? pc_ = addr : ++pc_;

	co_return;
}

CoObj Intel8080::CallOnFlag(bool status, std::string_view instructionName)
{
	timePeriods_ = status ? 17 : 11;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto addrLow = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto addr = Uint16(dataBus_->Receive(), addrLow);

	if constexpr (dbg == true)
	{
		printf("0x%04X %s 0x%04X\n", pc_ - 2, instructionName.data(), addr);
	}

	++pc_;
	
	if (status == true)
	{
		sp_ += 0xFFFF;
		WriteToAddress(Signal::MemoryWrite, sp_, pc_ >> 8);
		co_await coObj_;

		sp_ += 0xFFFF;
		WriteToAddress(Signal::MemoryWrite, sp_, pc_ & 0xFF);
		co_await coObj_;

		/*
			This needs to be moved ... by calling push above
			the pc_ will be incremented before the instruction completes.

			This works because we can't interrupt an instruction mid execution.
			If we could this would have to FIXED. It isn't technically correct, but works, a minor issue to fix someday.
		*/
		pc_ = addr;
	}

	co_return;
}

CoObj Intel8080::Push(const Register& hi, const Register low)
{
	if constexpr (dbg == true)
	{
		if ((opcode_ & 0x30) == 0x30)
		{
			printf("0x%04X PUSH PSW\n", pc_);
		}
		else
		{
			printf("0x%04X PUSH %c\n", pc_, registerName_[(opcode_ & 0x30) >> 3]);
		}
	}

	timePeriods_ = 11;

	sp_ += 0xFFFF;
	WriteToAddress(Signal::MemoryWrite, sp_, Value(hi));
	co_await coObj_;

	sp_ += 0xFFFF;
	WriteToAddress(Signal::MemoryWrite, sp_, Value(low));
	co_await coObj_;

	pc_++;
	co_return;
}

CoObj Intel8080::Adi(const Register& r)
{
	if constexpr (dbg == true)
	{
		printf("0x%04X ADI %c\n", pc_, registerName_[(opcode_ & 0x30) >> 3]);
	}

	timePeriods_ = 7;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	a_ = Value(a_) + dataBus_->Receive();
	++pc_;
	co_return;
}

#if 1
/**
	RST

	This section describes the RST (restart) instruction,
	which is a special purpose subroutine jump. This instruction
	occupies one byte.
	The contents of the program counter
	are pushed onto the stack, providing a return address for
	later use by a RETURN instruction.
*/
CoObj Intel8080::Rst(uint8_t restart)
{
	uint16_t addr = restart & 0x38;

	if constexpr (dbg == true)
	{
		printf("0x%04X INTERRUPT RST %d\n", pc_, addr >> 3);
	}

	timePeriods_ = 11;

	sp_ += 0xFFFF;
	WriteToAddress(Signal::MemoryWrite, sp_, pc_ >> 8);
	co_await coObj_;

	sp_ += 0xFFFF;
	WriteToAddress(Signal::MemoryWrite, sp_, pc_ & 0xFF);
	co_await coObj_;

	/*
		This needs to be moved ... by calling push above
		the pc_ will be incremented before the instruction completes.

		This works because we can't interrupt an instruction mid execution.
		If we could this would have to FIXED. It isn't technically correct, but works, a minor issue to fix someday.
	*/
	pc_ = addr;
	co_return;
}
#endif

CoObj Intel8080::Rst()
{
	//We need to increment pc_ before the call to Rst as the address of the next
	//instruction (++pc_) to be executed needs to be pushed to the stack so we
	//can return to it once the Rst completes.
	//++pc_;
	//Rst(opcode_);
	
	uint16_t addr = opcode_ & 0x38;

	if constexpr (dbg == true)
	{
		printf("0x%04X RST %d\n", pc_, addr >> 3);
	}

	timePeriods_ = 11;
	++pc_;

	sp_ += 0xFFFF;
	WriteToAddress(Signal::MemoryWrite, sp_, pc_ >> 8);
	co_await coObj_;

	sp_ += 0xFFFF;
	WriteToAddress(Signal::MemoryWrite, sp_, pc_ & 0xFF);
	co_await coObj_;

	/*
		This needs to be moved ... by calling push above
		the pc_ will be incremented before the instruction completes.

		This works because we can't interrupt an instruction mid execution.
		If we could this would have to FIXED. It isn't technically correct, but works, a minor issue to fix someday.
	*/
	pc_ = addr;

	co_return;
}

CoObj Intel8080::Out()
{
	timePeriods_ = 10;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto out = dataBus_->Receive();

	if constexpr (dbg == true)
	{
		printf("0x%04X OUT 0x%02X\n", pc_ - 1, out);
	}

	//write to IO port 'out' the accumulator
	WriteToAddress(Signal::IoWrite, out, Value(a_));
	co_await coObj_;
	++pc_;
	co_return;
}

CoObj Intel8080::In()
{
	timePeriods_ = 10;
	ReadFromAddress(Signal::MemoryRead, ++pc_);
	co_await coObj_;
	auto in = dataBus_->Receive();

	if constexpr (dbg == true)
	{
		printf("0x%04X IN 0x%02X\n", pc_ - 1, in);
	}

	//Read into the accumulator the value in IO port 'in'.
	ReadFromAddress(Signal::IoRead, in);
	co_await coObj_;
	a_ = dataBus_->Receive();

	++pc_;
	co_return;
}

/**
	XTHL

	The contents of the L register are exchanged with the contents of the memory byte whose address is held in the stack pointer SP.
	The contents of the H register are exchanged with the contents of the memory byte whose address is one greater than that held
	in the stack pointer.
*/
CoObj Intel8080::Xthl()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X XTHL\n", pc_);
	}

	timePeriods_ = 18;
	ReadFromAddress(Signal::MemoryRead, sp_);
	co_await coObj_;
	auto spl = dataBus_->Receive();
	ReadFromAddress(Signal::MemoryRead, sp_ + 1);
	co_await coObj_;
	auto sph = dataBus_->Receive();

	uint8_t l = Value(l_);
	uint8_t h = Value(h_);

	std::swap(spl, l);
	std::swap(sph, h);
	
	l_ = l;
	h_ = h;

	WriteToAddress(Signal::MemoryWrite, sp_, spl);
	co_await coObj_;
	WriteToAddress(Signal::MemoryWrite, sp_ + 1, sph);
	co_await coObj_;
	
	pc_++;
	co_return;
}

/**
	PCHL

	The contents of the H register replace the most significant 8 bits of the program counter,
	and the contents of the L register replace the least significant 8 bits of the program counter.
	This causes program execution to continue at the address contained in the H and L registers
*/
CoObj Intel8080::Pchl()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X PCHL\n", pc_);
	}

	pc_ = Uint16(h_, l_);
	timePeriods_ = 5;
	co_return;
}

CoObj Intel8080::Xchg()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X XCHG\n", pc_);
	}

	std::swap(h_, d_);
	std::swap(l_, e_);
	pc_++;
	timePeriods_ = 4;
	co_return;
}

/*
Implementation of the DI instruction resets the
interrupt flip - flop. This causes the computer to ignore
any subsequent interrupt signals.
*/
CoObj Intel8080::Di()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X DI\n", pc_);
	}

	iff_ = false;
	pc_++;
	timePeriods_ = 4;
	co_return;
}

CoObj Intel8080::Sphl()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X SPHL\n", pc_);
	}

	sp_ = Uint16(h_, l_);
	pc_++;
	timePeriods_ = 5;
	co_return;
}

/*
	Implementation of the EI instruction sets the
	interrupt flip-flop. This alerts the computer to the presence of interrupts and causes it to respond accordingly
*/
CoObj Intel8080::Ei()
{
	if constexpr (dbg == true)
	{
		printf("0x%04X EI\n", pc_);
	}

	iff_ = true;
	pc_++;
	timePeriods_ = 4;
	co_return;
}

}