#pragma once

// Float32 to float16 compressor
// Code from here: https://stackoverflow.com/a/3542975
// Used under the Unlicense: http://choosealicense.com/licenses/unlicense/

namespace adria
{

	class FloatCompressor
	{
		union Bits
		{
			Float f;
			Int32 si;
			Uint32 ui;
		};

		static constexpr Int shift = 13;
		static constexpr Int shiftSign = 16;

		static constexpr Int32 infN = 0x7F800000; // flt32 infinity
		static constexpr Int32 maxN = 0x477FE000; // max flt16 normal as a flt32
		static constexpr Int32 minN = 0x38800000; // min flt16 normal as a flt32
		static constexpr Int32 signN = 0x80000000; // flt32 sign bit

		static constexpr Int32 infC = infN >> shift;
		static constexpr Int32 nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
		static constexpr Int32 maxC = maxN >> shift;
		static constexpr Int32 minC = minN >> shift;
		static constexpr Int32 signC = signN >> shiftSign; // flt16 sign bit

		static constexpr Int32 mulN = 0x52000000; // (1 << 23) / minN
		static constexpr Int32 mulC = 0x33800000; // minN / (1 << (23 - shift))

		static constexpr Int32 subC = 0x003FF; // max flt32 subnormal down shifted
		static constexpr Int32 norC = 0x00400; // min flt32 normal down shifted

		static constexpr Int32 maxD = infC - maxC - 1;
		static constexpr Int32 minD = minC - subC - 1;

	public:

		static Uint16 Compress(Float value)
		{
			Bits v, s;
			v.f = value;
			Uint32 sign = v.si & signN;
			v.si ^= sign;
			sign >>= shiftSign; // logical shift
			s.si = mulN;
			s.si = static_cast<Int32>(s.f * v.f); // correct subnormals
			v.si ^= (s.si ^ v.si) & -(minN > v.si);
			v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
			v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
			v.ui >>= shift; // logical shift
			v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
			v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
			return static_cast<Uint16>(v.ui | sign);
		}

		static Float Decompress(Uint16 value)
		{
			Bits v;
			v.ui = value;
			Int32 sign = v.si & signC;
			v.si ^= sign;
			sign <<= shiftSign;
			v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
			v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
			Bits s;
			s.si = mulC;
			s.f *= v.si;
			Int32 mask = -(norC > v.si);
			v.si <<= shift;
			v.si ^= (s.si ^ v.si) & mask;
			v.si |= sign;
			return v.f;
		}
	};
}
