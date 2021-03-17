/******************************************************************************
 * File:        TableExtractionDemo.cpp
 * Description: A demonstration program for the table extraction api
 * Author:      Stefan Brechtken
 *
 * (C) Copyright 2021, Stefan Brechtken
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

#include <vector>
#include <iostream>
#include <string>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <tuple>
#include <memory>

#include <argp.h>

class Rectangle{
private:
  int xl; ///< left x coordinate
  int yt; ///< top y coordinate
  int xr; ///< right x coordinate
  int yb; ///< bottom y coordinate
public:
  Rectangle(const std::tuple<int, int, int, int>& r)
  {
    std::tie(xl, yt, xr, yb) = r;
  }
  Rectangle(int xl, int yt, int xr, int yb): xl(xl), yt(yt), xr(xr), yb(yb){}
  
  int width()   const {return abs(xr - xl);}
  int height()  const {return abs(yt - yb);}
  int area()    const {return width() * height();}
  
  Rectangle intersection(const Rectangle& r) const
  {
    const int overlap_xr = std::min<int>(xr, r.xr);
    const int overlap_xl = std::max<int>(xl, r.xl);
    const int overlap_yt = std::max<int>(yt, r.yt);
    const int overlap_yb = std::min<int>(yb, r.yb);
    if(overlap_xr > overlap_xl && overlap_yt < overlap_yb)
      return Rectangle(overlap_xl, overlap_yt, overlap_xr, overlap_yb);
    else
      return Rectangle(0, 0, 0, 0);
  }
  bool majorOverlap(const Rectangle& r) const
  {
    const Rectangle inter = intersection(r);
    if(inter.area() > 0.5f * std::min(area(), r.area()))
      return true;
    return false;
  }
  
  bool contains(int x, int y) const
  {
    return x > xl && x < xr && y > yt && y < yb;
  }
  
  void print() const {printf("%d, %d, %d, %d", xl, yt, xr, yb);}
};

std::string getTextInRegion(
  const Rectangle& region,
  tesseract::TessBaseAPI *api
)
{
  std::unique_ptr<tesseract::ResultIterator> ri(api->GetIterator());
  std::string text;
  if(ri)
  {
    do
    {
      do
      {
        std::unique_ptr<const char, void (*)(const char*)> textp(
          ri->GetUTF8Text(tesseract::RIL_WORD),
          [](const char* a) { delete[] a; });
        
        if(!textp)
          continue;
        
        int x1, y1, x2, y2;
        ri->BoundingBox(tesseract::RIL_WORD, &x1, &y1, &x2, &y2);
        const Rectangle word_area(x1, y1, x2, y2);
        
        if(region.majorOverlap(word_area))
          text += std::string(textp.get());

        if(ri->IsAtFinalElement(tesseract::RIL_TEXTLINE, tesseract::RIL_WORD))
          break;
      } while(ri->Next(tesseract::RIL_WORD));
    } while(ri->Next(tesseract::RIL_TEXTLINE));
  }
  
  return text;
}

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
  std::unique_ptr<const char, void (*)(const char*)> outText(
      api->GetUTF8Text(), [](const char* a) { delete[] a; });
  
  /// hold the program in order to inspect debug output
  std::cout<<"Tesseract recognition finished"<<std::endl<<std::endl;
  std::cout<<"Press any key to continue"<<std::endl; getchar();
  printf("OCR output:\n%s", outText.get());
  
  /// iterate over results
  std::unique_ptr<tesseract::ResultIterator> ri(api->GetIterator());
  tesseract::PageIteratorLevel level = tesseract::RIL_BLOCK;

  printf("available standard API:\n");
  if (ri != 0) {
    do {
      if( ri->BlockType() == tesseract::PT_TABLE )
      {
        int x1, y1, x2, y2;
        ri->BoundingBox(level, &x1, &y1, &x2, &y2);
        printf("table BoundingBox: %d,%d,%d,%d;\n", x1, y1, x2, y2);
      }
    } while (ri->Next(level));
  }
  
  printf("\nexperimental API for direct access to table detector results:\n");
  for(unsigned i = 0; i < api->GetNumberOfTables(); ++i)
  {
    int x1, y1, x2, y2;
    std::tie(x1, y1, x2, y2) = api->GetTableBoundingBox(i);
    std::vector<std::tuple<int,int,int,int>> rows = api->GetTableRows(i);
    std::vector<std::tuple<int,int,int,int>> cols = api->GetTableCols(i);
    
    printf("\ntable BoundingBox: %u x %u pos: %d, %d, %d, %d;\n",
           unsigned(rows.size()), unsigned(cols.size()), x1, y1, x2, y2);
    
    for(unsigned c = 0; c < cols.size(); ++c)
    {
      const Rectangle col(cols[c]);
      for(unsigned r = 0; r < rows.size(); ++r)
      {
        const Rectangle row(rows[r]);
        printf("Table %d, row %d, col %d, text \"%s\"\n", i, r, c,
              getTextInRegion(row.intersection(col), api).c_str());
      }
    }
    printf("\n");
  }
  
  api->End();
  pixDestroy(&image);
  delete api;
  return 0;
}
