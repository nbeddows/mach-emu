/*
Copyright (c) 2021-2023 Nicolas Beddows <nicolas.beddows@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

export module _8080;

import <array>;
import <bitset>;
import <functional>;
import <string_view>;

import Base;
import I8080;
import SystemBus;

//#define ENABLE_OPCODE_TABLE

namespace Emulator
{
	constexpr bool dbg = false;

	export class Intel8080 final : public I8080 //replace with public ICpu
	{
	private:
		using Register = std::bitset<8>;
		static constexpr uint8_t maxRegisters_ = 8;
		static constexpr char registerName_[maxRegisters_] = { 'B', 'C', 'D', 'E', 'H', 'L', 'M', 'A' };

		enum /*class*/Condition
		{
			CarryFlag = 0x00,
			ParityFlag = 0x02,
			AuxCarryFlag = 0x04,
			ZeroFlag = 0x06,
			SignFlag = 0x07
		};

		/**
			The 8080 provides the programmer with an 8-bit accumulator and six additional 8-bit "scratchpad" registers.
			These seven working registers are numbered and referenced via the integers 0, 1,2,3,4,5, and 7; by convention,
			these registers may also be accessed via the letters B, C, D,
			E, H, L, and A (for the accumulator), respectively.
		*/
		Register a_;
		Register b_;
		Register c_;
		Register d_;
		Register e_;
		Register h_;
		Register l_;
		//Helper funtions to prime the address and data buses
		//for read write operations.
		void ReadFromAddress(Signal readLocation, uint16_t addr);
		void WriteToAddress(Signal writeLocation, uint16_t addr, uint8_t value);

		/**
			The program counter is a 16 bit register which is accessible to the programmer and whose contents indicate the
			address of the next instruction to be executed.
		*/
		uint16_t pc_{};

		/**
			A stack is an area of memory set aside by the programmer in which data or addresses are stored and retrieved
			by stack operations. The programmer specifies which addresses the stack operations will
			operate upon via a special accessible 16-bit register called the stack pointer.
		*/
		uint16_t sp_{};

		/**
			Five condition (or status) bits are provided by the
			8080 to reflect the results of data operations

			S Z 0 AC 0 P 1 C
		*/
		Register status_ = 0b00000010;

		//interrupt flip-flip - 1 enabled, 0 disabled
		bool iff_{};

		//The opcode for the instruction to be executed.
		uint8_t opcode_{};
#ifdef ENABLE_OPCODE_TABLE 
		std::unique_ptr<std::function <uint8_t()>[]> opcodeTable_;
#endif		
		std::shared_ptr<AddressBus<uint16_t>> addressBus_;
		std::shared_ptr<DataBus<uint8_t>> dataBus_;
		std::shared_ptr<ControlBus<8>> controlBus_;

		static uint8_t Value(const Register& r) { return static_cast<uint8_t>(r.to_ulong()); }
		static uint16_t Uint16(const Register& hi, const Register& low) { return (Value(hi) << 8) | Value(low); }
		static bool AuxCarry(const Register& r, uint8_t value) { return ((Value(r) & 0x0F) + (value & 0x0F)) > 0x0F; }
		static bool Carry(const Register& r, uint8_t value) { return (Value(r) + value) > 0xFF; }
		static bool Parity(const Register& r) { return (r.count() & 1) == 0; }
		static bool Sign(const Register& r) { return r.test(7); }
		static bool Zero(const Register& r) { return r.none(); }

		//Should be implicitly inline
		inline uint8_t Inr(Register& r);
		inline uint8_t Inr();
		inline uint8_t Dcr(Register& r);
		inline uint8_t Dcr(uint16_t addr);
		inline uint8_t Mvi(Register& reg);
		inline uint8_t Mvi();
		inline uint8_t Daa();
		inline uint8_t Rlc();
		inline uint8_t Rrc();
		inline uint8_t Ral();
		inline uint8_t Rar();
		inline uint8_t Lxi(Register& regHi, Register& regLow);
		inline uint8_t Lxi();
		inline uint8_t Shld();
		inline uint8_t Stax(const Register& hi, const Register& low);
		inline uint8_t Inx(Register& hi, Register& low);
		inline uint8_t Inx();
		inline uint8_t Dad(const Register& hi, const Register& low);
		inline uint8_t Dad();
		inline uint8_t Lhld();
		inline uint8_t Ldax(const Register& hi, const Register& low);
		inline uint8_t Dcx(Register& hi, Register& low);
		inline uint8_t Dcx();
		inline uint8_t Cma();
		inline uint8_t Sta();
		inline uint8_t Stc();
		inline uint8_t Lda();
		inline uint8_t Cmc();
		inline uint8_t Mov(Register& lhs, const Register& rhs);
		inline uint8_t Mov(Register& lhs);
		inline uint8_t Mov(uint8_t value);
		inline uint8_t Nop();
		inline uint8_t Hlt();
		inline Register Add(const Register& lhs, const Register& rhs, uint8_t carry, bool setCarryFlag, [[maybe_unused]] std::string_view instructionName);
		inline uint8_t Add(const Register& r, std::string_view instructionName);
		inline uint8_t Add(uint16_t addr, std::string_view instructionName);
		inline uint8_t Adc(const Register& r, std::string_view instructionName);
		inline uint8_t Adc(uint16_t addr, std::string_view instructionName);
		inline Register Sub(const Register& r, uint8_t withCarry, std::string_view instructionName);
		inline uint8_t Sub(const Register& r, std::string_view instructionName);
		inline uint8_t Sub(uint16_t addr, std::string_view instructionName);
		inline uint8_t Sbb(const Register& r, std::string_view instructionName);
		inline uint8_t Sbb(uint16_t addr, std::string_view instructionName);
		inline void Ana(const Register& r);
		inline uint8_t Ana(const Register& r, std::string_view instructionName);
		inline uint8_t Ana(uint16_t addr, std::string_view instructionName);
		inline void Xra(const Register& r);
		inline uint8_t Xra(const Register& r, std::string_view instructionName);
		inline uint8_t Xra(uint16_t addr, std::string_view instructionName);
		inline void Ora(const Register& r);
		inline uint8_t Ora(const Register& r, std::string_view instructionName);
		inline uint8_t Ora(uint16_t addr, std::string_view instructionName);
		inline uint8_t Cmp(const Register& r, std::string_view instructionName);
		inline uint8_t Cmp(uint16_t addr, std::string_view instructionName);
		inline uint8_t NotImplemented();
		inline uint8_t RetOnFlag(bool status, std::string_view instructionName);
		inline uint8_t Pop(Register& hi, Register& low);
		inline uint8_t JmpOnFlag(bool status, std::string_view instructionName);
		inline uint8_t CallOnFlag(bool status, std::string_view instructionName);
		inline uint8_t Push(const Register& hi, const Register low);
		inline uint8_t Adi(const Register& r);
		inline uint8_t Rst();
		inline uint8_t Rst(uint8_t restart);
		inline uint8_t Out();
		inline uint8_t In();
		inline uint8_t Xthl();
		inline uint8_t Pchl();
		inline uint8_t Xchg();
		inline uint8_t Di();
		inline uint8_t Sphl();
		inline uint8_t Ei();
		uint8_t Fetch();
		std::function<void(const SystemBus<uint16_t, uint8_t, 8>&&)> process_;

	public:
		/* I8080 overrides */
		uint8_t Execute() override final;
		void Reset(uint16_t programCounter) override final;
		/* End I8080 overrides */

		Intel8080() = default;
		Intel8080 (const SystemBus<uint16_t, uint8_t, 8>& systemBus, std::function<void(const SystemBus<uint16_t, uint8_t, 8>&&)> process);
		~Intel8080() = default;

		uint8_t A() const { return Value(a_); }
		uint8_t B() const { return Value(b_); }
		uint8_t C() const { return Value(c_); }
		uint8_t D() const { return Value(d_); }
		uint8_t E() const { return Value(e_); }
		uint8_t H() const { return Value(h_); }
		uint8_t L() const { return Value(l_); }
		uint16_t SP() const { return sp_; }
		uint8_t Status() const { return Value(status_); }

		void PC(uint16_t pc) { pc_ = pc; }

		bool Zero() const { return status_[Condition::ZeroFlag]; }
		bool Sign() const { return status_[Condition::SignFlag]; }
		bool Parity() const { return status_[Condition::ParityFlag]; }
		bool AuxCarry() const { return status_[Condition::AuxCarryFlag]; }
		bool Carry() const { return status_[Condition::CarryFlag]; }
	};
}