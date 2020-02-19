#include <vector>
#include <iostream>
#include <string>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <tuple>

#include <argp.h>

class CmdArguments
{
  private:
    /// single argument parser invoked by argp
    static int parseOneArg(int key, char* arg, argp_state* state);
  
  public:
    std::string file_path;
    std::string data_path;
    std::string lang;
    std::string exec_path;
    
    /// parse (argc, argv) into CmdArguments
    CmdArguments(int argc, char** argv);
};

CmdArguments::CmdArguments(int argc, char** argv)
{
  exec_path = argv[0];

  // {long option, short option, argument, arg optional?, option description}<
  const argp_option option_list[] =
  {
    { "file-path", 'f', "/path/img", 0, "path to an image" },
    { "data-path", 'd', "/path/img", 0,
      "path to the tessdata folder containing the LSTM files" },
    { "language", 'l', "/path/img", 0, "language you want to use" },
    { 0 }
  };
  
  const argp setting {option_list, parseOneArg};
  argp_parse(&setting, argc, argv, 0, 0, this);
}

int CmdArguments::parseOneArg(int key, char* arg, argp_state* state)
{
  CmdArguments* data = (CmdArguments*) state->input;
  switch(key)
  {
    case 'f': data->file_path = arg; break;
    case 'd': data->data_path = arg; break;
    case 'l': data->lang = arg; break;
  }
  return 0;
}

int main(int argc, char** argv)
{
  CmdArguments cla(argc, argv); 
  if( cla.data_path.empty() )
    cla.data_path = "/usr/share/tesseract-ocr/4.00/tessdata/"; //ubuntu 18.04
  if( cla.lang.empty() )
    cla.lang = "eng";
  
  char *outText;

  tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
  
  if(api->Init(cla.data_path.c_str(), cla.lang.c_str(),
    tesseract::OcrEngineMode::OEM_LSTM_ONLY))
  {
      fprintf(stderr, "Could not initialize tesseract.\n");
      exit(1);
  }

  /// variables for table detection
  api-> SetVariable("textord_tabfind_find_tables", "true");
  api-> SetVariable("textord_tablefind_recognize_tables", "true");
  
  /// variables for table recognition debug output
//   api-> SetVariable("textord_show_tables", "true");
  
  /// Open input image with leptonica library
  Pix *image = pixRead(cla.file_path.c_str());
  api->SetPageSegMode(tesseract::PSM_AUTO);
  api->SetImage(image);
  /// Get OCR result
  outText = api->GetUTF8Text();
  /// hold the program in order to inspect debug output
  std::cout<<"Tesseract recognition finished"<<std::endl<<std::endl;
  std::cout<<"Press any key to continue"<<std::endl; getchar();
  printf("OCR output:\n%s", outText);
  
  /// iterate over results
  tesseract::ResultIterator* ri = api->GetIterator();
  tesseract::PageIteratorLevel level = tesseract::RIL_BLOCK;

  printf("old API:\n");
  if (ri != 0) {
    do {
      if( ri->BlockType() == PT_TABLE )
      {
        int x1, y1, x2, y2;
        ri->BoundingBox(level, &x1, &y1, &x2, &y2);
        printf("\ntable BoundingBox: %d,%d,%d,%d;\n", x1, y1, x2, y2);
      }
    } while (ri->Next(level));
  }
  
  printf("\nexperimental API:\n");
  for(unsigned i = 0; i < api->GetNumberOfTables(); ++i)
  {
    int x1, y1, x2, y2;
    std::tie(x1, y1, x2, y2) = api->GetTableBoundingBox(i);
    printf("table BoundingBox: %d, %d, %d, %d;\n", x1, y1, x2, y2);
    std::vector<std::tuple<int,int,int,int>> rows = api->GetTableRows(i);
    std::vector<std::tuple<int,int,int,int>> cols = api->GetTableCols(i);
    
    for(const std::tuple<int, int, int, int>& t: rows)
    {
      std::tie(x1, y1, x2, y2) = t;
      printf("row: %d, %d, %d, %d;\n", x1, y1, x2, y2);
    }
    printf("\n");
    for(const std::tuple<int, int, int, int>& t: cols)
    {
      std::tie(x1, y1, x2, y2) = t;
      printf("col: %d, %d, %d, %d;\n", x1, y1, x2, y2);
    }
    printf("\n");
  }
  
  api->End();
  delete [] outText;
  pixDestroy(&image);
  delete api;
    return 0;
}
