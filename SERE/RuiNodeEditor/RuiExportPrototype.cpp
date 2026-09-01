#include "RuiNodeEditor/RuiExportPrototype.h"


RuiExportPrototype::RuiExportPrototype(const RenderInstance& inst,const std::string& name):size(inst.elementWidth,inst.elementHeight),name(name) {
	renderJobCount = 0;
}

void RuiExportPrototype::AddConstant(float f) {
	if (floatConstants.contains(f))return;
	floatConstants.emplace(f, currentDataStructSize);
	currentDataStructSize += 4;
}

void RuiExportPrototype::AddConstant(std::string s) {
	if (stringConstants.contains(s))return;
	if (currentDataStructSize % 8)currentDataStructSize += 8 - (currentDataStructSize % 8);
	stringConstants.emplace(s, currentDataStructSize);
	currentDataStructSize += 8;
}

void RuiExportPrototype::AddIntConstant(int v) {
	if (intConstants.contains(v))return;
	intConstants.emplace(v, currentDataStructSize);
	currentDataStructSize += 4;
}

void RuiExportPrototype::AddTransformData(uint8_t* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		transformData.push_back(data[i]);
	}
}

void RuiExportPrototype::AddRenderJobData(uint8_t* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		renderJobData.push_back(data[i]);
	}
}

void RuiExportPrototype::AddDataVariable(const FloatVariable& var) {
	if (var.IsConstant()) {
		AddConstant(var.value);
	}
	else {
		varsInDataStruct.emplace(var.name, VariableType::FLOAT);
	}
}

void RuiExportPrototype::AddDataVariable(const Float2Variable& var) {
	if (var.IsConstant()) {
		AddConstant(var.value.x);
		AddConstant(var.value.y);
	}
	else {
		varsInDataStruct.emplace(var.name, VariableType::FLOAT2);
	}
}

void RuiExportPrototype::AddDataVariable(const ColorVariable& var) {
	if (var.IsConstant()) {
		AddConstant(var.value.red);
		AddConstant(var.value.green);
		AddConstant(var.value.blue);
		AddConstant(var.value.alpha);
	}
	else {
		varsInDataStruct.emplace(var.name, VariableType::COLOR_ALPHA);
	}
}

void RuiExportPrototype::AddDataVariable(const AssetVariable& var) {
	if (!var.name.empty())
		varsInDataStruct.emplace(var.name, VariableType::ASSET_HANDLE);
}

void RuiExportPrototype::AddDataVariable(const StringVariable& var) {
	if (var.IsConstant()) {
		AddConstant(var.value);
	}
	else {
		varsInDataStruct.emplace(var.name, VariableType::STRING);
	}
}

void RuiExportPrototype::AddDataVariable(const IntVariable& var) {
	if (var.IsConstant()) {
		AddIntConstant(var.value);
	}
	else {
		varsInDataStruct.emplace(var.name, VariableType::INT);
	}
}

uint16_t RuiExportPrototype::GetFloatDataVariableOffset(const FloatVariable& var) {
	if (var.IsConstant())
		return GetFloatConstantOffset(var.value);
	return varOffsets[var.name];
}

Float2Offsets RuiExportPrototype::GetFloat2DataVariableOffset(const Float2Variable& var) {
	Float2Offsets res;
	if (var.IsConstant()) {
		res.x = GetFloatConstantOffset(var.value.x);
		res.y = GetFloatConstantOffset(var.value.y);
		return res;
	}
	res.x = varOffsets[var.name];
	res.y = res.x + 4;
	return res;
}

Float3Offsets RuiExportPrototype::GetFloat3DataVariableOffset(const Float3Variable& var) {
	Float3Offsets res;
	if (var.IsConstant()) {
		res.x = GetFloatConstantOffset(var.value.x);
		res.y = GetFloatConstantOffset(var.value.y);
		res.z = GetFloatConstantOffset(var.value.z);
		return res;
	}
	res.x = varOffsets[var.name];
	res.y = res.x + 4;
	res.z = res.y + 4;
	return res;
}

ColorOffsets RuiExportPrototype::GetColorDataVariableOffset(const ColorVariable& var) {
	ColorOffsets res;
	if (var.IsConstant()) {
		res.red = GetFloatConstantOffset(var.value.red);
		res.green = GetFloatConstantOffset(var.value.green);
		res.blue = GetFloatConstantOffset(var.value.blue);
		res.alpha = GetFloatConstantOffset(var.value.alpha);
		return res;
	}
	res.red = varOffsets[var.name];
	res.green = res.red + 4;
	res.blue = res.green + 4;
	res.alpha = res.blue + 4;
	return res;
}

uint16_t RuiExportPrototype::GetAssetDataVariableOffset(const AssetVariable& var) {
	return varOffsets[var.name];
}

uint16_t RuiExportPrototype::GetIntDataVariableOffset(const IntVariable& var) {
	if (var.IsConstant())
		return GetIntConstantOffset(var.value);
	return varOffsets[var.name];
}

uint16_t RuiExportPrototype::GetStringDataVariableOffset(const StringVariable& var) {
	if (var.IsConstant())
		return GetStringConstantOffset(var.value);
	return varOffsets[var.name];
}


uint16_t RuiExportPrototype::GetFloatConstantOffset(float f) {
	assert(floatConstants.contains(f) && "Float not found in Float Constants");
	return floatConstants[f];
}

uint16_t RuiExportPrototype::GetStringConstantOffset(std::string s) {
	assert(stringConstants.contains(s) && "Float not found in Float Constants");
	return stringConstants[s];
}

uint16_t RuiExportPrototype::GetIntConstantOffset(int v) {
	assert(intConstants.contains(v) && "Int not found in Int Constants");
	return intConstants[v];
}

void RuiExportPrototype::GenerateCode() {
	std::set<std::string> existingVariables;
	for (auto& [arg, _] : arguments) {
		existingVariables.emplace(arg);
	}

	codeLines.push_back(std::format("extern \"C\" __declspec(dllexport) void {}(RuiFunctions_t* funcs,RuiGlobals* globals,RuiInstance* inst,{}_data* data){{",name,name));
	codeLines.push_back("__m128* transformSize = funcs->GetTransformSize(inst);");
	size_t codeElementsHit = 0;
	bool addedVar = true;
	while (addedVar) {
		addedVar = false;
		for (auto& ele : codeElements) {
			if (existingVariables.contains(ele.identifier))continue;
			bool dependencyMissing = false;
			for (auto& dep : ele.dependencys) {
				if (!dep.size())continue;
				if (!existingVariables.contains(dep)) {
					dependencyMissing = true;
					break;
				}
			}
			if (dependencyMissing)continue;
			ele.callback(*this);
			existingVariables.insert(ele.identifier);
			addedVar = true;
			codeElementsHit++;
		}
	}
	if (codeElementsHit < codeElements.size()) {
		printf("Not all code has been exported should be %llu was %llu\n",codeElements.size(),codeElementsHit);
		for (auto& ele : codeElements) {
			if (existingVariables.find(ele.identifier) == existingVariables.end()) {
				printf("could not add %s missing",ele.identifier.c_str());
				for (auto& dep : ele.dependencys) {
					if (existingVariables.find(dep) == existingVariables.end()) {
						printf(" %s",dep.c_str());
					}
				}
				printf("\n");
			}
		}
		printf("Error done");
	}
		

	codeLines.push_back(std::format("funcs->executeTransform(inst,{});",transformData.size()));
	codeLines.push_back("}");

}

void RuiExportPrototype::GenerateTransformData() {
	// The engine pre-initializes running_index to 3 (after the 3 base transforms
	// at indices 0, 1, 2).  Real DLL functions emit NO type-1 copy entry;
	// user opcode-2 entries start writing directly to output[3+].
	// A type-1 copy would advance running_index to 6, causing opcode-2 to write
	// to the wrong output slots while widgets still reference indices 3+.

	printf("[SERE] GenerateTransformData: %zu callbacks to process\n", transformCallbacks.size());
	printf("[SERE] Initial transformIndices: %zu entries\n", transformIndices.size());
	for (auto& [hash, idx] : transformIndices)
		printf("[SERE]   idx=%d hash=0x%llX\n", idx, (unsigned long long)hash);

	bool addedVar = true;
	int pass = 0;
	while (addedVar) {
		addedVar = false;
		pass++;
		for (auto& ele : transformCallbacks) {
			if (transformIndices.contains(ele.identifier))continue;
			bool dependencyMissing = false;
			for (auto& dep : ele.dependencys) {
				if (!transformIndices.contains(dep)) {
					dependencyMissing = true;
					printf("[SERE]   Pass %d: callback id=0x%llX BLOCKED by dep=0x%llX\n", pass, (unsigned long long)ele.identifier, (unsigned long long)dep);
					break;
				}
			}
			if (dependencyMissing)continue;
			printf("[SERE]   Pass %d: callback id=0x%llX FIRED\n", pass, (unsigned long long)ele.identifier);
			ele.callback(*this);
			addedVar = true;
		}
	}
	printf("[SERE] GenerateTransformData done: %zu indices, %zu bytes\n", transformIndices.size(), transformData.size());
}

void RuiExportPrototype::GenerateRenderJobData() {
	std::sort(renderJobs.begin(), renderJobs.end(), [](ExportRenderJob& a, ExportRenderJob& b) {
		return a.layer<b.layer;
	});
	for (auto& job : renderJobs) {
		job.func(*this);
	}
#ifdef _DEBUG
	printf("[SERE Export Debug] Float constants (%zu):\n", floatConstants.size());
	for (auto& [val, off] : floatConstants) {
		printf("  offset 0x%04X = %f\n", off, val);
	}
	printf("[SERE Export Debug] Style descriptors (%zu):\n", styleDescriptor.size());
	for (size_t i = 0; i < styleDescriptor.size(); i++) {
		auto& sd = styleDescriptor[i];
		printf("  [%zu] type=%d color0=(%04X,%04X,%04X,%04X) tint=(%04X,%04X,%04X,%04X) blend=%04X premul=%04X\n",
			i, sd.type,
			sd.color0.red, sd.color0.green, sd.color0.blue, sd.color0.alpha,
			sd.tint[0], sd.tint[1], sd.tint[2], sd.tint[3],
			sd.blend, sd.premul);
	}
#endif
}

// Engine UI_FindVarMeta: seed=varType (0 for ARG), per-char
// hash = (hash >> 20) ^ (bias + hash * scale + c); slot = hash & (n-1); validation = hash >> 4.
uint32_t calculateShortHash(const char* name, uint32_t mul, uint32_t add) {
	uint32_t hash = 0;
	while (*name) {
		const unsigned char c = static_cast<unsigned char>(*name++);
		hash = (hash >> 20) ^ (add + hash * mul + c);
	}
	return hash;
}

void RuiExportPrototype::GenerateArguments() {

	cluster.argCount = 1;
	cluster.argIndex = 0;
	cluster.short_6 = 0;
	cluster.valueSize = (uint16_t)defaultValues.size();
	cluster.dataStructSize = currentDataStructSize;
	cluster.short_C = 1;
	cluster.short_E = 0;
	cluster.renderJobCount = renderJobCount;
	while (cluster.argCount < arguments.size())cluster.argCount *= 2;

	for (int add = 0; add < 256; add++) {
		bool success = true;
		for (int mul = 1; mul < 256; mul++) {
			success = true;
			std::vector<bool> argSlots(cluster.argCount, false);
			for (auto& [name,type] : arguments) {
				uint32_t argIndex = calculateShortHash(name.c_str(), mul, add) & (cluster.argCount - 1);
				if (argSlots[argIndex]) {
					success = false;
					break;
				}
				argSlots[argIndex] = true;
			}
			if (success) {
				cluster.byte_4 = mul;
				cluster.byte_5 = add;
				break;
			}
		}
		if(success)break;
	}
	exportArgs.resize(cluster.argCount);
	for (auto& [name, type] : arguments) {
		uint32_t hash = calculateShortHash(name.c_str(),cluster.byte_4,cluster.byte_5);
		uint32_t index = hash & (cluster.argCount -1);
		exportArgs[index].type = type;
		exportArgs[index].dataOffset = varOffsets[name];
		exportArgs[index].nameOffset = 0;
		exportArgs[index].shortHash = hash >> 4;
		
	}
}

void RuiExportPrototype::GenerateVariables(std::map<std::string,std::any>& argValues) {
	defaultValues.resize(currentDataStructSize);
	for (auto& [value, offset] : floatConstants) {
		*(float*)&defaultValues[offset] = value;
	}
	for (auto& [value, offset] : intConstants) {
		*(int*)&defaultValues[offset] = value;
	}
	for (auto& [value, offset] : stringConstants) {
		*(uint64_t*)&defaultValues[offset] = (uint64_t)defaultStrings.str().size();
		rpakPointersInDefaultValues.push_back(offset);
		defaultStrings.write(value.c_str(),value.size());
		defaultStrings.put(0);
	}
	for (auto& [name, type] : arguments) {
		size_t size;
		switch (type) {
		case VariableType::STRING:
		case VariableType::ASSET:
		case VariableType::IMAGE:
		case VariableType::FONT_FACE:
		{
			if (currentDataStructSize % 8) {
				currentDataStructSize += 8 - (currentDataStructSize % 8);
			}
			size = 8;
			defaultValues.resize(currentDataStructSize + size);
			*(size_t*)&defaultValues[currentDataStructSize] = defaultStrings.str().size();
			rpakPointersInDefaultValues.push_back(currentDataStructSize);
			std::string val = std::any_cast<std::string>(argValues[name]);
			defaultStrings.write(val.c_str(), val.size());
			defaultStrings.put(0);
		}
		break;
		case VariableType::INT:
		case VariableType::BOOL:
		case VariableType::UIHANDLE:
		case VariableType::FONT_HASH:
			size = 4;
			defaultValues.resize(currentDataStructSize + size);
			*(int*)&defaultValues[currentDataStructSize] = std::any_cast<int>(argValues[name]);
			break;
		case VariableType::FLOAT:
		case VariableType::GAMETIME:
			size = 4;
			defaultValues.resize(currentDataStructSize + size);
			*(float*)&defaultValues[currentDataStructSize] = std::any_cast<float>(argValues[name]);
			break;
		case VariableType::WALLTIME:
		case VariableType::ARRAY:
			if (currentDataStructSize % 8) {
				currentDataStructSize += 8 - (currentDataStructSize % 8);
			}
			size = 8;
			defaultValues.resize(currentDataStructSize + size);
			if (argValues[name].type() == typeid(uint64_t))
				*(uint64_t*)&defaultValues[currentDataStructSize] = std::any_cast<uint64_t>(argValues[name]);
			else
				*(uint64_t*)&defaultValues[currentDataStructSize] = 0;
			break;
		case VariableType::FLOAT2:
			size = 8;
			defaultValues.resize(currentDataStructSize + size);
			*(Vector2*)&defaultValues[currentDataStructSize] = std::any_cast<Vector2>(argValues[name]);
			break;
		case VariableType::FLOAT3:
			size = 12;
			defaultValues.resize(currentDataStructSize + size);
			*(Vector3*)&defaultValues[currentDataStructSize] = std::any_cast<Vector3>(argValues[name]);
			break;
		case VariableType::COLOR_ALPHA:
			size = 16;
			defaultValues.resize(currentDataStructSize + size);
			*(Color*)&defaultValues[currentDataStructSize] = std::any_cast<Color>(argValues[name]);
			break;
		default:
			size = 0;
			break;
		}

		varOffsets.emplace(name, currentDataStructSize);
		currentDataStructSize += (uint16_t)size;
	}


	for (auto& [name, type] : varsInDataStruct) {
		if(name.empty() || varOffsets.contains(name))
			continue;
		if ((type == VariableType::STRING || type == VariableType::FONT_FACE
			|| type == VariableType::WALLTIME || type == VariableType::ARRAY)
			&& (currentDataStructSize % 8)) {
			currentDataStructSize += 8 - (currentDataStructSize % 8);
		}
		varOffsets.emplace(name, currentDataStructSize);
		size_t size;
		switch (type) {
		case VariableType::INT:
		case VariableType::BOOL:
		case VariableType::FLOAT:
		case VariableType::GAMETIME:
		case VariableType::ASSET_HANDLE:
		case VariableType::UIHANDLE:
		case VariableType::FONT_HASH:
			size = 4;
			break;
		case VariableType::FLOAT2:
		case VariableType::STRING:
		case VariableType::WALLTIME:
		case VariableType::ARRAY:
		case VariableType::FONT_FACE:
			size = 8;
			break;
		case VariableType::FLOAT3:
			size = 12;
			break;
		case VariableType::COLOR_ALPHA:
			size = 16;
			break;
		default:
			size = 0;
			break;
		}
		currentDataStructSize += size;
	}


}

bool RuiExportPrototype::GenerateCodeStruct() {
	codeLines.push_back(std::format("struct {}_data{{",name));
	
	// Count valid (non-empty-named) variables
	size_t validVarCount = 0;
	for (auto& [name, offset] : varOffsets)
		if (!name.empty()) validVarCount++;

	if (validVarCount == 0) {
		if (currentDataStructSize > 0)
			codeLines.push_back(std::format("_BYTE constants[{}];",currentDataStructSize));
	}
	else {

		size_t currentOffset = 0;
		size_t addedVars = 0;
		while (addedVars < validVarCount) {
			std::string lowestVar;
			for (auto& [name, offset] : varOffsets) {
				if (name.empty()) continue;
				if (offset >= currentOffset) {
					lowestVar = name;
					break;
				}
			}
			if (lowestVar.empty()) break;
			for (auto& [name, offset] : varOffsets) {
				if (name.empty()) continue;
				if (offset < varOffsets[lowestVar] && offset >= currentOffset)
					lowestVar = name;
			}
			VariableType type = VariableType::NONE;
			if(arguments.contains(lowestVar))
				type = arguments[lowestVar];
			else if(varsInDataStruct.contains(lowestVar))
				type = varsInDataStruct[lowestVar];
			if(type == VariableType::NONE)
				break;
			if (currentOffset < varOffsets[lowestVar]) {
				codeLines.push_back(std::format("_BYTE pad_{}[{}];",lowestVar,varOffsets[lowestVar]-currentOffset));
				currentOffset = varOffsets[lowestVar];
			}
			switch (type) {
			case VariableType::BOOL:
			case VariableType::INT:
			case VariableType::UIHANDLE:
			case VariableType::FONT_HASH:
				codeLines.push_back(std::format("int {};",lowestVar));
				currentOffset +=4;
				break;
			case VariableType::FLOAT:
			case VariableType::GAMETIME:
				codeLines.push_back(std::format("float {};",lowestVar));
				currentOffset +=4;
				break;
			case VariableType::WALLTIME:
			case VariableType::ARRAY:
				codeLines.push_back(std::format("uint64_t {};",lowestVar));
				currentOffset +=8;
				break;
			case VariableType::FLOAT2:
				codeLines.push_back(std::format("Vector2 {};",lowestVar));
				currentOffset +=8;
				break;
			case VariableType::FLOAT3:
				codeLines.push_back(std::format("Vector3 {};",lowestVar));
				currentOffset +=12;
				break;
			case VariableType::COLOR_ALPHA:
				codeLines.push_back(std::format("Color {};",lowestVar));
				currentOffset +=16;
				break;
			case VariableType::STRING:
			case VariableType::ASSET:
			case VariableType::IMAGE:
			case VariableType::FONT_FACE:
				codeLines.push_back(std::format("const char* {};",lowestVar));
				currentOffset +=8;
				break;
			case VariableType::ASSET_HANDLE:
				codeLines.push_back(std::format("uint32_t {};",lowestVar));
				currentOffset +=4;
				break;
			}
			addedVars++;
		}

	}
	codeLines.push_back("};");
}

void RuiExportPrototype::Generate(std::unordered_map<ImFlow::NodeUID, std::shared_ptr<ImFlow::BaseNode>>& nodes, RenderInstance& render) {
	for (int i = 0; i < 3; i++)
		transformIndices.emplace(render.transformResults[i].hash, i);

	for (auto& [uid, node] : nodes) {
		std::dynamic_pointer_cast<RuiBaseNode>(node)->Export(*this);
	}
	GenerateVariables(render.arguments);
	GenerateTransformData();
	GenerateRenderJobData();
	GenerateArguments();
	GenerateCodeStruct();
	GenerateCode();
}


void RuiExportPrototype::WriteToFile(fs::path path) {
	std::ofstream file(path,std::ios::binary);
	if(!file.good())return;

	RuiPackageHeader_v2_t pkgHdr{};
	pkgHdr.magic = 'R' | 'U' << 8 | 'I' << 16 | 'P' << 24;
	pkgHdr.packageVersion = 2;
	// S21 engine walks V42.1-class widget sizes; live ui.rpak stamps asset version 42.
	pkgHdr.ruiVersion = 42;
	pkgHdr.elementWidth = size.x;
	pkgHdr.elementWidthRcp = NRReciprocal(size.x);
	pkgHdr.elementHeight = size.y;
	pkgHdr.elementHeightRcp = NRReciprocal(size.y);
	pkgHdr.nameSize = (uint16_t)(name.size() + 1);
	pkgHdr.defaultValuesSize = (uint16_t)defaultValues.size();
	pkgHdr.dataStructSize = currentDataStructSize;
	pkgHdr.defaultStringsSize = (uint32_t)defaultStrings.str().size();
	pkgHdr.defaultStringsDataSize = defaultStrings.str().size();
	pkgHdr.styleDescriptorCount = (uint16_t)styleDescriptor.size();
	pkgHdr.renderJobSize = (uint32_t)renderJobData.size();
	pkgHdr.transformDataSize = (uint16_t)transformData.size();
	pkgHdr.rpakPointersInDefaltDataCount = (uint16_t)rpakPointersInDefaultValues.size();
	pkgHdr.keyframingCount = 0;
	pkgHdr.keyframingSize = 0;
	pkgHdr.renderJobCount = renderJobCount;
	pkgHdr.argClusterCount = 1;
	pkgHdr.argCount = cluster.argCount;
	// maxTransformIndex: highest transform result index used
	pkgHdr.unk_A4 = transformIndices.empty() ? 0 : (uint16_t)transformIndices.size();
	pkgHdr.argNamesSize = 0;

	// v2 fixups: string slots in defaultValues point into defaultStrings (CPU section).
	// Section 1 = combined defaultValues+defaultStrings blob in repak (offset = slot value).
	pointerFixups.clear();
	for (uint16_t ptrOffset : rpakPointersInDefaultValues) {
		PointerFixup_t fx{};
		fx.srcSection = 1;
		fx.srcOffset = ptrOffset;
		fx.dstSection = 1;
		uint64_t strOff = 0;
		if (ptrOffset + sizeof(uint64_t) <= defaultValues.size())
			memcpy(&strOff, defaultValues.data() + ptrOffset, sizeof(uint64_t));
		fx.dstOffset = (uint32_t)strOff;
		pointerFixups.push_back(fx);
	}
	pkgHdr.pointerFixupCount = (uint32_t)pointerFixups.size();

	// Write placeholder header (will rewrite with correct offsets at end)
	file.write((char*)&pkgHdr, sizeof(pkgHdr));

	// v2 section order: name, argNames, argClusters, arguments,
	// styleDescriptors, renderJobs, keyframings, transformData,
	// defaultValues, defaultStrings, rpakPointers, pointerFixups

	pkgHdr.nameOffset = file.tellp();
	file.write(name.c_str(), name.size());
	file.put(0);

	pkgHdr.argNamesOffset = file.tellp();
	// argNames is empty for SERE-generated assets

	pkgHdr.argClusterOffset = file.tellp();
	file.write((char*)&cluster, sizeof(cluster));

	pkgHdr.argumentsOffset = file.tellp();
	file.write((char*)exportArgs.data(), exportArgs.size() * sizeof(Argument_t));

	pkgHdr.styleDescriptorOffset = file.tellp();
	file.write((char*)styleDescriptor.data(), styleDescriptor.size() * sizeof(StyleDescriptorOffsets));

	pkgHdr.renderJobOffset = file.tellp();
	file.write((char*)renderJobData.data(), renderJobData.size());

	pkgHdr.keyframingOffset = file.tellp();
	// keyframings is empty for SERE-generated assets

	pkgHdr.transformDataOffset = file.tellp();
	file.write((char*)transformData.data(), transformData.size());

	pkgHdr.defaultValuesOffset = file.tellp();
	file.write((char*)defaultValues.data(), defaultValues.size());

	pkgHdr.defaultStringDataOffset = file.tellp();
	file.write((char*)defaultStrings.str().c_str(), defaultStrings.str().size());

	pkgHdr.rpakPointersInDefaultDataOffset = file.tellp();
	file.write((char*)rpakPointersInDefaultValues.data(), rpakPointersInDefaultValues.size() * sizeof(uint16_t));

	pkgHdr.pointerFixupOffset = file.tellp();
	file.write((char*)pointerFixups.data(), pointerFixups.size() * sizeof(PointerFixup_t));

	// Rewrite header with correct offsets
	file.seekp(0);
	file.write((char*)&pkgHdr, sizeof(pkgHdr));

	file.close();

	// Write the .cpp companion file
	fs::path cppPath = path;
	cppPath.replace_extension("cpp");
	std::ofstream codeFile(cppPath);
	if(!codeFile.good())
		return;
	const std::string import = "#include \"RuiHeaders.h\"";
	codeFile.write(import.c_str(), import.size());
	codeFile.put('\n');
	for (auto& line : codeLines) {
		codeFile.write(line.c_str(), line.size());
		codeFile.put('\n');
	}
	codeFile.close();

	// Auto-generate RuiHeaders.h next to the .cpp
	fs::path headerPath = cppPath.parent_path() / "RuiHeaders.h";
	std::ofstream hdr(headerPath);
	if (!hdr.good())
		return;
	hdr << "#pragma once\n";
	hdr << "// Auto-generated by SERE - do not edit manually\n";
	hdr << "#include <stdint.h>\n";
	hdr << "#include <stddef.h>\n";
	hdr << "#include <cmath>\n";
	hdr << "#include <immintrin.h>\n\n";
	hdr << "typedef unsigned char _BYTE;\n";
	hdr << "typedef uint64_t _QWORD;\n";
	hdr << "typedef uint32_t _DWORD;\n";
	hdr << "typedef uint16_t _WORD;\n";
	hdr << "struct _OWORD { uint64_t lo; uint64_t hi; };\n\n";

	// Always emit all types (RuiGlobals may reference Vector3)
	hdr << "struct Vector2 { float x, y; Vector2()=default; Vector2(float x_,float y_):x(x_),y(y_){} };\n";
	hdr << "struct Vector3 { float x, y, z; Vector3()=default; Vector3(float x_,float y_,float z_):x(x_),y(y_),z(z_){} };\n";
	hdr << "struct Color { float r, g, b, a; Color()=default; Color(float r_,float g_,float b_,float a_):r(r_),g(g_),b(b_),a(a_){} };\n";
	hdr << "\n";

	hdr << "struct RuiInstance;\n\n";

	// RuiGlobals - always emit full struct (nodes may reference globals fields)
	bool needsGlobals = true;
	if (needsGlobals) {
		hdr << "// UiScriptSysVars_s (272 bytes) — engine globals passed to ruiFunc\n";
		hdr << "struct RuiGlobals {\n";
		hdr << "\t// --- Transform & Camera (0x00-0x47) ---\n";
		hdr << "\tchar _localFromWorld[0x30];      // +0x00: local-from-world 4x3 matrix (48B)\n";
		hdr << "\tVector3 camOrgLocal;             // +0x30: camera origin (local space)\n";
		hdr << "\tVector3 localPlayerPos;          // +0x3C: camera origin (world space) / player position\n";
		hdr << "\n\t// --- Viewport (0x48-0x4F) ---\n";
		hdr << "\tfloat screenWidth;               // +0x48: viewportSize.x\n";
		hdr << "\tfloat screenHeight;              // +0x4C: viewportSize.y\n";
		hdr << "\n\t// --- View-Projection Matrix (0x50-0x8F) ---\n";
		hdr << "\tchar _viewProjMtx[0x40];         // +0x50: view-projection 4x4 matrix (64B)\n";
		hdr << "\n\t// --- Time (0x90-0x9F) ---\n";
		hdr << "\tuint64_t wallTime;               // +0x90: real wall clock time\n";
		hdr << "\tfloat currentTime;               // +0x98: in-game time (gameTime)\n";
		hdr << "\tfloat uiTime;                    // +0x9C: UI animation time\n";
		hdr << "\n\t// --- Kill Replay (0xA0-0xA7) ---\n";
		hdr << "\tfloat killReplayChangeTime;      // +0xA0: kill replay change timestamp\n";
		hdr << "\tint killReplayIsWatching;        // +0xA4: 1 if watching kill replay\n";
		hdr << "\n\t// --- Player State (0xA8-0xDB) ---\n";
		hdr << "\tint playerIsUsingController;     // +0xA8\n";
		hdr << "\tint playerIsAlive;               // +0xAC\n";
		hdr << "\tint playerIsSpectator;           // +0xB0\n";
		hdr << "\tint playerHasOpenMenu;           // +0xB4\n";
		hdr << "\tint playerIsPhaseShifted;        // +0xB8\n";
		hdr << "\tint playerIsOneHanded;           // +0xBC\n";
		hdr << "\tfloat globalAdsFrac;             // +0xC0: aim down sights fraction (0.0-1.0)\n";
		hdr << "\tint playerSniperScopeEquipped;   // +0xC4\n";
		hdr << "\tint playerIsViewingDeathScreen;  // +0xC8\n";
		hdr << "\tint playerIsPureSpectator;       // +0xCC\n";
		hdr << "\tint playerIsThirdPerson;         // +0xD0\n";
		hdr << "\tint playerIsDrivingHoverVehicle; // +0xD4\n";
		hdr << "\tfloat playerCrosshairADSFrac;    // +0xD8\n";
		hdr << "\n\t// --- Game State (0xDC-0x107) ---\n";
		hdr << "\tVector3 gameFriendlyTeamColor;   // +0xDC: friendly team color RGB\n";
		hdr << "\tVector3 gameEnemyTeamColor;      // +0xE8: enemy team color RGB\n";
		hdr << "\tVector3 gamePartyTeamColor;      // +0xF4: party team color RGB\n";
		hdr << "\tfloat gameAnnouncementChangeTime; // +0x100\n";
		hdr << "\tint gameAnnouncementIsActive;    // +0x104\n";
		hdr << "\n\t// --- Misc (0x108-0x10F) ---\n";
		hdr << "\tint nxMode;                      // +0x108: Nintendo Switch mode\n";
		hdr << "\tint _padding;                    // +0x10C\n";
		hdr << "};\n\n";
	}
	else {
		hdr << "struct RuiGlobals;\n\n";
	}

	// S21 native UI_* table, 48 slots. Slot 0 kills the instance; slot 1 only hides it.
	hdr << "struct RuiFunctions_t {\n";
	hdr << "\tvoid  (*Die)(RuiInstance* inst);\n";
	hdr << "\tvoid  (*SetHidden)(RuiInstance* inst);\n";
	hdr << "\tvoid  (*SetErrorWithReason)(RuiInstance* inst, const char* msg);\n";
	hdr << "\t__m128* (*GetTransformSize)(RuiInstance* inst);\n";
	hdr << "\t__m128 (*GetTextSize)(RuiInstance* inst, int renderJobByteOffset, int cacheIdx);\n";
	hdr << "\tvoid* GetGroupSize;\n";
	hdr << "\tvoid  (*executeTransform)(RuiInstance* inst, int transformDataSize);\n";
	hdr << "\tvoid  (*executeTransformAndResize)(RuiInstance* inst, int transformDataSize);\n";
	hdr << "\tconst char* (*SNPrintF)(RuiInstance* inst, const char* fmt, ...);\n";
	hdr << "\tconst char* (*Localize)(RuiInstance* inst, const char* key, int64_t a3, int64_t a4, int64_t a5, int64_t a6, int64_t a7);\n";
	hdr << "\tconst char* (*ToUpper)(RuiInstance* inst, char* text);\n";
	hdr << "\tconst char* (*FormatNumber)(RuiInstance* inst, const char* fmt, float n);\n";
	hdr << "\tconst char* (*LocalizeNumber)(RuiInstance* inst, float number);\n";
	hdr << "\tconst char* (*FormatAndLocalizeNumber)(RuiInstance* inst, const char* fmt, float number);\n";
	hdr << "\tbool  (*IsLanguageRTL)();\n";
	hdr << "\tbool  (*IsLanguageCJK)();\n";
	hdr << "\t__m128 (*SrgbToLinear)(__m128 c);\n";
	hdr << "\tfloat (*SinNorm)(RuiInstance* inst, float val);\n";
	hdr << "\tfloat (*Atan2Norm)(RuiInstance* inst, float y, float x);\n";
	hdr << "\tfloat (*RandomFloat)(RuiInstance* inst, float min, float max);\n";
	hdr << "\t__m128 (*ProjectWorldPoint)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\t__m128 (*LetterboxSize)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\t__m128 (*NestedUiSize)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tint32_t (*LoadAsset)(RuiInstance* inst, int prevValue, const char* imageName, uint64_t optionalHash);\n";
	hdr << "\tconst char* (*ImageToIconString)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tfloat (*Animate1D)(RuiInstance* inst, int animIdx, float time);\n";
	hdr << "\t__m128 (*Animate2D)(RuiInstance* inst, int animIdx, float time);\n";
	hdr << "\t__m128 (*Animate3D)(RuiInstance* inst, int animIdx, float time);\n";
	hdr << "\t__m128 (*Animate4D)(RuiInstance* inst, int animIdx, float time);\n";
	hdr << "\t__m128 (*NestedUiFinalSize)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tuint16_t (*UiFontNameHash)(const char* str);\n";
	hdr << "\t__m128 (*GetKeyColor)(RuiInstance* inst, int colorId, int mode);\n";
	hdr << "\tfloat (*GetViewportScale)(RuiInstance* inst);\n";
	hdr << "\t__m128 (*NestedUiAutoSize)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tfloat (*FlexGetRemainingSpace)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tfloat (*FlexGetWidgetSize)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tfloat (*EstimateTextWidth)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\t__m128 (*ProjectionHelper)(int64_t a1, int64_t a2, __m128 a3, __m128 a4, __m128* a5);\n";
	hdr << "\tfloat (*GetArrowRotation)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tvoid  (*Msg)();\n";
	hdr << "\tuint32_t (*HashString)(const char* str);\n";
	hdr << "\tconst char* (*GetLocString0)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tconst char* (*GetLocString1)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tconst char* (*GetLocString2)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tconst char* (*GetLocString3)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tconst char* (*GetLocString4)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tconst char* (*GetLocString5)(RuiInstance* inst, int64_t a2);\n";
	hdr << "\tconst char* (*GetLocString6)(RuiInstance* inst, int64_t a2);\n";
	hdr << "};\n";
	hdr << "static_assert(offsetof(RuiFunctions_t, GetTextSize) == 0x20);\n";
	hdr << "static_assert(offsetof(RuiFunctions_t, executeTransform) == 0x30);\n";
	hdr << "static_assert(offsetof(RuiFunctions_t, NestedUiSize) == 0xB0);\n";
	hdr << "static_assert(offsetof(RuiFunctions_t, LoadAsset) == 0xB8);\n";
	hdr << "static_assert(offsetof(RuiFunctions_t, HashString) == 0x140);\n";
	hdr << "static_assert(offsetof(RuiFunctions_t, GetLocString0) == 0x148);\n";
	hdr << "static_assert(sizeof(RuiFunctions_t) == 0x180);\n";
	hdr.close();
}

bool RuiExportPrototype::hasTypeInCodeStruct(VariableType type) const {
	for (auto& [name, t] : arguments)
		if (t == type) return true;
	for (auto& [name, t] : varsInDataStruct)
		if (t == type) return true;
	return false;
}