#include "WrapperGenerator.h"

using namespace std;

WrapperGenerator::WrapperGenerator(shared_ptr<unordered_map<string, 
                                   FunctionPrototype>> _prototypeMap,
                                   std::shared_ptr<std::unordered_map<std::string, 
                                   LogInformation>> _logInfoMap,
                                   string pathPrefix):
                                   prototypeMap(_prototypeMap),
                                   logInfoMap(_logInfoMap) {
    headerFile.open(pathPrefix + "VProfEventWrappers.h");
    implementationFile.open(pathPrefix + "VProfEventWrappers.cpp");

    const string generatedFileMessage = 
        " ///////////////////////////////////////////////////// \n"
        " // Note that this file was generated by VProfiler. // \n"
        " // Please do not change the contents of this file! // \n"
        " ///////////////////////////////////////////////////// \n\n";

    headerFile << generatedFileMessage; 
    implementationFile << generatedFileMessage;
}

vector<string> WrapperGenerator::getFilenames() {
    vector<string> result;

    for (auto kv : *prototypeMap) {
        result.push_back(kv.second.filename);
    }

    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    return result;
}

void WrapperGenerator::GenerateHeader() {
    vector<string> includeNames = getFilenames();
    for (string &includeName : includeNames) {
        headerFile << "#include \"" + includeName + "\"\n";
    }
    headerFile << "#include \"TraceTool.h\"\n\n";

    for (auto kv : *prototypeMap) {
        headerFile << kv.second.functionPrototype + ";\n\n";
    }

    headerFile.close();
}

void WrapperGenerator::GenerateImplementations() {
    implementationFile << "#include \"VProfEventWrappers.h\"\n\n";

    for (auto kv : *prototypeMap) {
        implementationFile << kv.second.functionPrototype + " {\n\t";

        if (kv.second.returnType != "void") {
            implementationFile << kv.second.returnType + " result;\n\t";
        }

        implementationFile << "SynchronizationTraceTool::LogEventStart(" + 
                              to_string((*logInfoMap)[kv.first].functionID) + ", " 
                              + ((*logInfoMap)[kv.first].isMessageBased ? "true" : "false")  
                              + ");\n\t";

        if (kv.second.returnType != "void") {
            implementationFile << "result = ";
        }

        implementationFile << kv.second.innerCallPrefix + "(";

        for (int i = 0, j = kv.second.paramVars.size(); i < j; i++) {
            implementationFile << kv.second.paramVars[i];

            if (i != (j - 1)) {
                implementationFile << ", ";
            }
        }

        implementationFile <<");\n\t";

        implementationFile << "EventTraceTool::LogEventEnd(" + 
                              to_string((*logInfoMap)[kv.first].functionID) + ", " 
                              + ((*logInfoMap)[kv.first].isMessageBased ? "true" : "false")  
                              + ");\n";

        if (kv.second.returnType != "void") {
            implementationFile << "\treturn result;\n";
        }

        implementationFile << "}\n\n";
    }

    implementationFile.close();
}

void WrapperGenerator::GenerateWrappers() {
    GenerateHeader();
    GenerateImplementations();
}
