/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// wasm2asm console tool
//

#include <iostream>
#include <map>
#include <string>

#include "emscripten-optimizer/istring.h"
#include "parsing.h"
#include "support/colors.h"
#include "support/command-line.h"
#include "support/file.h"
#include "support/threads.h"
#include "support/utilities.h"
#include "wasm-io.h"
#include "wasm-printing.h"
#include "wasm.h"

using namespace cashew;
using namespace wasm;

int main(int argc, const char* argv[]) {
  std::string sourceMapFilename;
  Options options("wasm-dis",
                  "Un-assemble a .wasm (WebAssembly binary format) into a "
                  ".wat (WebAssembly text format)");
  options
    .add("--output",
         "-o",
         "Output file (stdout if not specified)",
         Options::Arguments::One,
         [](Options* o, const std::string& argument) {
           o->extra["output"] = argument;
           Colors::setEnabled(false);
         })
    .add(
      "--source-map",
      "-sm",
      "Consume source map from the specified file to add location information",
      Options::Arguments::One,
      [&sourceMapFilename](Options* o, const std::string& argument) {
        sourceMapFilename = argument;
      })
    .add_positional("INFILE",
                    Options::Arguments::One,
                    [](Options* o, const std::string& argument) {
                      o->extra["infile"] = argument;
                    });
  options.parse(argc, argv);

  if (options.debug) {
    std::cerr << "parsing binary..." << std::endl;
  }
  Module wasm;
  try {
    ModuleReader().readBinary(options.extra["infile"], wasm, sourceMapFilename);
  } catch (ParseException& p) {
    p.dump(std::cerr);
    std::cerr << '\n';
    Fatal() << "error in parsing wasm binary";
  } catch (MapParseException& p) {
    p.dump(std::cerr);
    std::cerr << '\n';
    Fatal() << "error in parsing wasm source mapping";
  }

  if (options.debug) {
    std::cerr << "Printing..." << std::endl;
  }
  Output output(options.extra["output"], Flags::Text);
  WasmPrinter::printModule(&wasm, output.getStream());
  output << '\n';

  if (options.debug) {
    std::cerr << "Done." << std::endl;
  }
}
