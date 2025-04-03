import proto_parser
import proto_generator

import argparse
import sys
import os


if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("-output", action="store", dest="output_path", default="", nargs=1)
    args = arg_parser.parse_args(sys.argv[1:])

    print("Running protobuf generate Qt common types")
    print(sys.argv[1:])

    output_path = os.path.normpath(args.output_path[0])

    parser = proto_parser.Parser([proto_parser.get_well_known_types_path()])
    proto_generator.generate_qt_object_files(parser, [proto_parser.get_well_known_types_path()], output_path)
    proto_generator.generate_data_type_models(output_path, proto_parser.data_types)
