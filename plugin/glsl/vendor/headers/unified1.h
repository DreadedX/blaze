// Copyright (c) 2017 The Khronos Group Inc.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and/or associated documentation files (the "Materials"),
// to deal in the Materials without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Materials, and to permit persons to whom the
// Materials are furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Materials.
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS KHRONOS
// STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS SPECIFICATIONS AND
// HEADER INFORMATION ARE LOCATED AT https://www.khronos.org/registry/ 
// 
// THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS
// IN THE MATERIALS.

#ifndef SPIRV_EXTINST_unified1_H_
#define SPIRV_EXTINST_unified1_H_

#ifdef __cplusplus
extern "C" {
#endif

enum { unified1Version = 100, unified1Version_BitWidthPadding = 0x7fffffff };
enum { unified1Revision = 1, unified1Revision_BitWidthPadding = 0x7fffffff };

enum unified1Instructions {
    unified1DebugInfoNone = 0,
    unified1DebugCompilationUnit = 1,
    unified1DebugTypeBasic = 2,
    unified1DebugTypePointer = 3,
    unified1DebugTypeQualifier = 4,
    unified1DebugTypeArray = 5,
    unified1DebugTypeVector = 6,
    unified1DebugTypedef = 7,
    unified1DebugTypeFunction = 8,
    unified1DebugTypeEnum = 9,
    unified1DebugTypeComposite = 10,
    unified1DebugTypeMember = 11,
    unified1DebugTypeInheritance = 12,
    unified1DebugTypePtrToMember = 13,
    unified1DebugTypeTemplate = 14,
    unified1DebugTypeTemplateParameter = 15,
    unified1DebugTypeTemplateTemplateParameter = 16,
    unified1DebugTypeTemplateParameterPack = 17,
    unified1DebugGlobalVariable = 18,
    unified1DebugFunctionDeclaration = 19,
    unified1DebugFunction = 20,
    unified1DebugLexicalBlock = 21,
    unified1DebugLexicalBlockDiscriminator = 22,
    unified1DebugScope = 23,
    unified1DebugNoScope = 24,
    unified1DebugInlinedAt = 25,
    unified1DebugLocalVariable = 26,
    unified1DebugInlinedVariable = 27,
    unified1DebugDeclare = 28,
    unified1DebugValue = 29,
    unified1DebugOperation = 30,
    unified1DebugExpression = 31,
    unified1DebugMacroDef = 32,
    unified1DebugMacroUndef = 33,
    unified1InstructionsMax = 0x7ffffff
};


enum unified1DebugInfoFlags {
    unified1FlagIsProtected = 0x01,
    unified1FlagIsPrivate = 0x02,
    unified1FlagIsPublic = 0x03,
    unified1FlagIsLocal = 0x04,
    unified1FlagIsDefinition = 0x08,
    unified1FlagFwdDecl = 0x10,
    unified1FlagArtificial = 0x20,
    unified1FlagExplicit = 0x40,
    unified1FlagPrototyped = 0x80,
    unified1FlagObjectPointer = 0x100,
    unified1FlagStaticMember = 0x200,
    unified1FlagIndirectVariable = 0x400,
    unified1FlagLValueReference = 0x800,
    unified1FlagRValueReference = 0x1000,
    unified1FlagIsOptimized = 0x2000,
    unified1DebugInfoFlagsMax = 0x7ffffff
};

enum unified1DebugBaseTypeAttributeEncoding {
    unified1Unspecified = 0,
    unified1Address = 1,
    unified1Boolean = 2,
    unified1Float = 4,
    unified1Signed = 5,
    unified1SignedChar = 6,
    unified1Unsigned = 7,
    unified1UnsignedChar = 8,
    unified1DebugBaseTypeAttributeEncodingMax = 0x7ffffff
};

enum unified1DebugCompositeType {
    unified1Class = 0,
    unified1Structure = 1,
    unified1Union = 2,
    unified1DebugCompositeTypeMax = 0x7ffffff
};

enum unified1DebugTypeQualifier {
    unified1ConstType = 0,
    unified1VolatileType = 1,
    unified1RestrictType = 2,
    unified1DebugTypeQualifierMax = 0x7ffffff
};

enum unified1DebugOperation {
    unified1Deref = 0,
    unified1Plus = 1,
    unified1Minus = 2,
    unified1PlusUconst = 3,
    unified1BitPiece = 4,
    unified1Swap = 5,
    unified1Xderef = 6,
    unified1StackValue = 7,
    unified1Constu = 8,
    unified1DebugOperationMax = 0x7ffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_EXTINST_unified1_H_
