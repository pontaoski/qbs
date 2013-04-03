import qbs 1.0

Product {
    name: "blubb_header"
    type: "hpp"
    files: "blubb_header.h.in"
    property string blubbProp: project.blubbProp

    Transformer {
        Artifact {
            fileName: "blubb_header.h"
            fileTags: "hpp"
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generating blubb_header.h";
            cmd.highlight = "codegen";
            cmd.blubbProp = product.blubbProp;
            cmd.sourceCode = function() {
                file = new TextFile(output.fileName, TextFile.WriteOnly);
                file.truncate();
                file.write("#define BLUBB_PROP " + blubbProp);
                file.close();
            }
            return cmd;
        }
    }

    ProductModule {
        Depends { name: "cpp" }
        cpp.includePaths: product.buildDirectory
    }
}
