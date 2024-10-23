#include "args.h"
#include <exception>
#include "open_save.h"
#include <iostream>
#include "pipeline.h"

#include "blur.h"
#include "crop.h"
#include "edge_dedection.h"
#include "glass.h"
#include "greyscale.h"
#include "negative.h"
#include "sharpening.h"

Pipeline::Factory CreatePipelineFactory() {
    Pipeline::Factory result{};
    result.RegisterFilterFactory<Blur::Factory>("-blur");
    result.RegisterFilterFactory<Crop::Factory>("-crop");
    result.RegisterFilterFactory<Greyscale::Factory>("-gs");
    result.RegisterFilterFactory<Negative::Factory>("-neg");
    result.RegisterFilterFactory<Sharpening::Factory>("-sharp");
    result.RegisterFilterFactory<EdgeDetection::Factory>("-edge");
    result.RegisterFilterFactory<Glass::Factory>("-glass");
    return result;
}

void PrintHelp(std::ostream& output, std::string_view exec_name) {
    output << "Usage: " << exec_name << " "
           << "{input file} {output file} "
           << "[-{filter name 1} [filter param 1] [filter param 2] ...] "
           << "[-{filter name 2} [filter param 1] [filter param 2] ...] "
           << "..." << std::endl;
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        PrintHelp(std::cout, argv[0]);
        return 0;
    }
    try {
        Args args(argc, argv);
        Image image{};
        image.Read(args.GetInputFile().c_str());

        Pipeline::Factory pipeline_factory = CreatePipelineFactory();
        Pipeline pipeline{};

        for (const Args::Filter& filter : args.GetFilters()) {
            pipeline.AddFilter(pipeline_factory.CreateFilter(filter.GetName(), filter.GetParameters()));
        }
        pipeline.Process(image);
        SaveFile(args.GetOutputFile().c_str(), image);

    } catch (std::exception& exception) {
        std::cerr << "Error:" << exception.what() << std::endl;
        PrintHelp(std::cout, argv[0]);
        return 1;
    }
    return 0;
}
