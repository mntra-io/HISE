/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/


namespace snex {
namespace mir {
using namespace juce;


LoopManager::~LoopManager()
{
	jassert(labelPairs.isEmpty());
}

void LoopManager::pushLoopLabels(String& startLabel, String& endLabel, String& continueLabel)
{
	startLabel = makeLabel();
	endLabel = makeLabel();
	continueLabel = makeLabel();

	labelPairs.add({ startLabel, endLabel, continueLabel });
}

String LoopManager::getCurrentLabel(const String& instructionId)
{
	if (instructionId == "continue")
		return labelPairs.getLast().continueLabel;
	else if (instructionId == "break")
		return labelPairs.getLast().endLabel;

	jassertfalse;
	return "";
}

void LoopManager::popLoopLabels()
{
	labelPairs.removeLast();
}

String LoopManager::makeLabel() const
{
	return "L" + String(labelCounter++);
}

String TextLine::addAnonymousReg(MIR_type_t type, RegisterType rt)
{
	auto& rm = state->registerManager;

	auto id = rm.getAnonymousId(MIR_fp_type_p(type) && rt == RegisterType::Value);
	rm.registerCurrentTextOperand(id, type, rt);

	auto t = TypeInfo(TypeConverters::MirType2TypeId(type), true);

	auto tn = TypeConverters::TypeInfo2MirTextType(t);

	if (rt == RegisterType::Pointer || type == MIR_T_P)
		tn = "i64";

	localDef << tn << ":" << id;
	return id;
}

TextLine::TextLine(State* s_):
	state(s_)
{

}

TextLine::~TextLine()
{
	jassert(flushed);
}

void TextLine::addImmOperand(const VariableStorage& value)
{
	operands.add(Types::Helpers::getCppValueString(value));
}

void TextLine::addSelfAsValueOperand()
{
	operands.add(state->registerManager.getOperandForChild(-1, RegisterType::Value));
}

void TextLine::addSelfAsPointerOperand()
{
	operands.add(state->registerManager.getOperandForChild(-1, RegisterType::Pointer));
}

void TextLine::addChildAsPointerOperand(int childIndex)
{
	operands.add(state->registerManager.loadIntoRegister(childIndex, RegisterType::Pointer));
}

void TextLine::addChildAsValueOperand(int childIndex)
{
	operands.add(state->registerManager.loadIntoRegister(childIndex, RegisterType::Value));
}

void TextLine::addOperands(const Array<int>& indexes, const Array<RegisterType>& registerTypes /*= {}*/)
{
	if (!registerTypes.isEmpty())
	{
		jassert(indexes.size() == registerTypes.size());

		for (int i = 0; i < indexes.size(); i++)
			operands.add(state->registerManager.getOperandForChild(indexes[i], registerTypes[i]));
	}
	else
	{
		for (auto& i : indexes)
			operands.add(state->registerManager.getOperandForChild(i, RegisterType::Value));
	}
}

int TextLine::getLabelLength() const
{
	return label.isEmpty() ? -1 : (label.length() + 2);
}

String TextLine::toLine(int maxLabelLength) const
{
	String s;

	if (label.isNotEmpty())
		s << label << ":" << " ";

	auto numToAdd = maxLabelLength - s.length();

	for (int i = 0; i < numToAdd; i++)
		s << " ";

	if (!localDef.isEmpty())
		s << "local " << localDef << "; ";


	s << instruction;

	if (!operands.isEmpty())
	{
		s << " ";

		auto numOps = operands.size();
		auto index = 0;

		for (auto& o : operands)
		{
			s << o;

			if (++index < numOps)
				s << ", ";
		}
	}

	if (comment.isNotEmpty())
		s << " # " << comment;

	return s;
}

void TextLine::flush()
{
	if (!flushed)
	{
		flushed = true;
		state->lines.add(*this);
	}
}

void FunctionManager::addPrototype(State* state, const FunctionData& f, bool addObjectPointer)
{
	TextLine l(state);

	l.label = "proto" + String(prototypes.size());
	l.instruction = "proto";

	if (f.returnType.isValid())
		l.operands.add(TypeConverters::TypeInfo2MirTextType(f.returnType));

	if (addObjectPointer)
	{
		l.operands.add("i64:_this_");
	}

	for (const auto& a : f.args)
		l.operands.add(TypeConverters::Symbol2MirTextSymbol(a));

	l.flush();

	prototypes.add(f);
}

bool FunctionManager::hasPrototype(const FunctionData& sig) const
{
	auto thisLabel = TypeConverters::FunctionData2MirTextLabel(sig);

	for (const auto& p : prototypes)
	{
		auto pLabel = TypeConverters::FunctionData2MirTextLabel(p);

		if (pLabel == thisLabel)
			return true;
	}

	return false;
}

String FunctionManager::getPrototype(const FunctionData& sig) const
{
	auto thisLabel = TypeConverters::FunctionData2MirTextLabel(sig);

	int l = 0;

	for (const auto& p : prototypes)
	{
		auto pLabel = TypeConverters::FunctionData2MirTextLabel(p);

		if (pLabel == thisLabel)
		{
			return "proto" + String(l);
		}

		l++;
	}

	throw String("prototype not found");
}

void DataManager::setDataLayout(const String& b64)
{
	dataList = SyntaxTreeExtractor::getDataLayoutTrees(b64);

	for (const auto& l : dataList)
	{
		Array<MemberInfo> members;

		for (const auto& m : l)
		{
			if (m.getType() == Identifier("Member"))
			{
				auto id = m["ID"].toString();
				auto type = Types::Helpers::getTypeFromTypeName(m["type"].toString());
				auto mir_type = TypeConverters::TypeInfo2MirType(TypeInfo(type));
				auto offset = (size_t)(int)m["offset"];

				members.add({ id, mir_type, offset });
			}
		}

		classTypes.emplace(Identifier(l["ID"].toString()), std::move(members));
	}
}

void DataManager::startClass(const Identifier& id, Array<MemberInfo>&& memberInfo)
{
	classTypes.emplace(id, memberInfo);
	numCurrentlyParsedClasses++;
}

void DataManager::endClass()
{
	numCurrentlyParsedClasses--;
}

juce::ValueTree DataManager::getDataObject(const String& type) const
{
	for (const auto& s : dataList)
	{
		if (s["ID"].toString() == type)
		{
			return s;
		}
	}

	return {};
}

size_t DataManager::getNumBytesRequired(const Identifier& id)
{
	size_t numBytes = 0;

	for (const auto&m : classTypes[id])
		numBytes = m.offset + Types::Helpers::getSizeForType(TypeConverters::MirType2TypeId(m.type));

	return numBytes;
}

const juce::Array<snex::mir::MemberInfo>& DataManager::getClassType(const Identifier& id)
{
	return classTypes[id];
}

RegisterManager::RegisterManager(State* state_):
	state(state_)
{

}

String RegisterManager::getAnonymousId(bool isFloat) const
{
	String s;
	s << (isFloat ? "xmm" : "reg") << String(counter++);
	return s;
}

void RegisterManager::emitMultiLineCopy(const String& targetPointerReg, const String& sourcePointerReg, int numBytesToCopy)
{
	auto use64 = numBytesToCopy % 8 == 0;

	jassert(use64);

	for (int i = 0; i < numBytesToCopy; i += 8)
	{
		TextLine l(state);
		l.instruction = "mov";

		auto dst = "i64:" + String(i) + "(" + targetPointerReg + ")";
		auto src = "i64:" + String(i) + "(" + sourcePointerReg + ")";

		l.operands.add(dst);
		l.operands.add(src);
		l.flush();
	}
}

int RegisterManager::allocateStack(const String& targetName, int numBytes, bool registerAsCurrentStatementReg)
{
	TextLine l(state);

	if (registerAsCurrentStatementReg)
		registerCurrentTextOperand(targetName, MIR_T_I64, RegisterType::Pointer);

	l.localDef << "i64:" << targetName;

	static constexpr int Alignment = 16;

	auto numBytesToAllocate = numBytes + (Alignment - numBytes % Alignment);

	l.instruction = "alloca";
	l.operands.add(targetName);
	l.addImmOperand((int)numBytesToAllocate);
	l.flush();

	return numBytesToAllocate;
}

void RegisterManager::registerCurrentTextOperand(String n, MIR_type_t type, RegisterType rt)
{
	if (isParsingFunction())
		localOperands.add({ state->currentTree, n, {}, type, rt });
	else
		globalOperands.add({ state->currentTree, n, {}, type, rt });
}

String RegisterManager::loadIntoRegister(int childIndex, RegisterType targetType)
{
	if (getRegisterTypeForChild(childIndex) == RegisterType::Pointer ||
		getTypeForChild(childIndex) == MIR_T_P)
	{
		auto t = getTypeForChild(childIndex);

		TextLine load(state);
		load.instruction = "mov";

		auto id = "p" + String(counter++);

		load.localDef << "i64:" << id;
		load.operands.add(id);
		load.addOperands({ childIndex }, { RegisterType::Pointer });
		load.flush();

		if (targetType == RegisterType::Pointer)
			return id;
		else
		{
			String ptr;

			auto ptr_t = TypeConverters::MirType2MirTextType(t);

			if (ptr_t == "i64")
				ptr_t = "i32";

			ptr << ptr_t << ":(" << id << ")";
			return ptr;
		}
	}
	else
		return getOperandForChild(childIndex, RegisterType::Value);
}

snex::mir::TextOperand RegisterManager::getTextOperandForValueTree(const ValueTree& c)
{
	for (const auto& t : localOperands)
	{
		if (t.v == c)
			return t;
	}

	for (const auto& t : globalOperands)
	{
		if (t.v == c)
			return t;
	}

	throw String("not found");
}

snex::mir::RegisterType RegisterManager::getRegisterTypeForChild(int index)
{
	auto t = getTextOperandForValueTree(state->getCurrentChild(index));
	return t.registerType;
}

MIR_type_t RegisterManager::getTypeForChild(int index)
{
	auto t = getTextOperandForValueTree(state->getCurrentChild(index));
	return t.type;
}

String RegisterManager::getOperandForChild(int index, RegisterType requiredType)
{
	auto t = getTextOperandForValueTree(state->getCurrentChild(index));

	if (t.registerType == RegisterType::Pointer && requiredType == RegisterType::Value)
	{
		auto x = t.text;

		if (t.stackPtr.isNotEmpty())
		{
			x = t.stackPtr;
		}

		auto vt = TypeConverters::MirType2MirTextType(t.type);

		// int registers are always 64 bit, but the 
		// address memory layout is 32bit integers so 
		// we need to use a different int type when accessing pointers
		if (vt == "i64")
			vt = "i32";

		vt << ":(" << x << ")";

		return vt;
	}
	else
	{
		return t.stackPtr.isNotEmpty() ? t.stackPtr : t.text;
	}
}



String State::operator[](const Identifier& id) const
{
	if (!currentTree.hasProperty(id))
	{
		dump();
		throw String("No property " + id.toString());
	}

	return currentTree[id].toString();
}

void State::dump() const
{
	DBG(currentTree.createXml()->createDocument(""));
}

void State::emitSingleInstruction(const String& instruction, const String& label /*= {}*/)
{
	TextLine l(this);
	l.instruction = instruction;
	l.label = label;
	l.flush();
}

void State::emitLabel(const String& label)
{
	TextLine noop(this);
	noop.label = label;
	noop.instruction = "bt";
	noop.operands.add(label);
	noop.addImmOperand(0);
	noop.appendComment("noop");
	noop.flush();
}

String State::toString(bool addTabs)
{
	String s;

	if (!addTabs)
	{
		for (const auto& l : lines)
			s << l.toLine(-1) << "\n";
	}
	else
	{
		int maxLabelLength = -1;

		for (const auto& l : lines)
			maxLabelLength = jmax(maxLabelLength, l.getLabelLength());

		for (const auto& l : lines)
			s << l.toLine(maxLabelLength) << "\n";
	}

	return s;
}

juce::ValueTree State::getCurrentChild(int index)
{
	if (index == -1)
		return currentTree;
	else
		return currentTree.getChild(index);
}

void State::processChildTree(int childIndex)
{
	ScopedValueSetter<ValueTree> svs(currentTree, currentTree.getChild(childIndex));
	auto ok = processTreeElement(currentTree);

	if (ok.failed())
		throw ok.getErrorMessage();
}

void State::processAllChildren()
{
	for (int i = 0; i < currentTree.getNumChildren(); i++)
		processChildTree(i);
}

juce::Result State::processTreeElement(const ValueTree& v)
{
	try
	{
		currentTree = v;
		return instructionManager.perform(this, v);
	}
	catch (String& e)
	{
		return Result::fail(e);
	}

	return Result::fail("unknown value tree type " + v.getType());
}

juce::Result InstructionManager::perform(State* state, const ValueTree& v)
{
	if (auto f = instructions[v.getType()])
	{
		return f(state);
	}

	throw String("no instruction found");
}

}
}