//////////////////////////////////////////////////////////////////
// This program read the output of Ocean View and make histograms.
// I saved the histogram in a root output for future analysis.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <map>
#include <stdio.h>
#include <math.h>

#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"


using namespace std;

class RefSpectrum{
public:
	RefSpectrum(){};
	~RefSpectrum(){};

	vector<float> XValue;
	std::vector<std::vector<float>> dataSets; 
	std::map<float, vector<float>> dataMap;
	vector<float> YValue;
	vector<float> YSTDDEV;


	void AssignSpectrum();
	void CalcMean();
	void CalcSTDDEV();
};

void RefSpectrum::AssignSpectrum(){
	assert(XValue.size() > 0);
	for (int i = 0; i<XValue.size(); i++){
		vector<float> singleDist;
		for (int j = 0; j < dataSets.size(); j++){
			singleDist.push_back(dataSets.at(j).at(i));
		}
		dataMap.insert(make_pair(XValue.at(i), singleDist));
	}
}

void RefSpectrum::CalcMean(){
	assert(dataMap.size() >0);
	for (auto& x: dataMap){
		float mean = 0;
		float sum = 0;
		for (int i = 0; i < x.second.size(); i++){
			sum += x.second.at(i);
		}
		mean = sum/x.second.size();
		YValue.push_back(mean);
		std::cout<<"mean: "<<mean<<std::endl;
	}
}

void RefSpectrum::CalcSTDDEV(){
	assert(YValue.size() > 0);
	for (int j = 0; j < XValue.size(); j++){
		float cx = XValue.at(j);
		float DEV = 0;
    for (int i = 0; i < dataMap.at(cx).size(); i++){
      DEV += (dataMap.at(cx).at(i)-YValue.at(cx))*(dataMap.at(cx).at(i)-YValue.at(cx));
    }
    YSTDDEV.push_back(sqrt(DEV/dataMap.size()));
  }
}

void OVAnalysis(string argv){
  string fileList= argv;  // The name of the input file list.
  ifstream inputList(fileList.c_str());
  cout<<"Reading the file list "<< argv <<endl;
  assert(inputList.is_open());

  string OutputFile(fileList); // Prepare the name of output .root file.
  string OutputFigure(fileList); // Prepare the name of output .pdf file.
  size_t listPos = fileList.find(".list");
  OutputFile.replace(listPos, 5, "-Processed.root");
  OutputFigure.replace(listPos, 5, ".pdf");
  cout<<"Prepared output file: "<<OutputFile<<endl;
  TFile* gOutputFile = TFile::Open(OutputFile.c_str(), "RECREATE");

  string inputFileName; //Prepare the name of
	double count = 0;
	RefSpectrum dataSpectrum;

	// read each file in the list
  while (inputList>>inputFileName){
    if (inputList.fail()) break;
    if (inputFileName[0] == '#') continue;
    if(inputFileName.find("break") != string::npos) break;
    ifstream inputFile(inputFileName.c_str());
    assert(inputFile.is_open());
    cout<<"Reading input file: "<< inputFileName << ".." << std::endl;

    string testString;  // The string that will be copied from entire lines of data file.
    //vector<float> XValue;  // The vector that stores all the x values.
    //std::vector<std::vector<float>> dataSets;  // The vector that stores multiple datasets

    // Start reading data.
    while (getline(inputFile, testString)){
      //Look for only the lines that contains data.
      if (testString == ">>>>>Begin Spectral Data<<<<<") {
				// setup x-axis values
        string xString;
        getline(inputFile, xString);
        istringstream xstream(xString);
        while (xstream) {
          float xEle = 0;
          xstream >> xEle;
          if (!xstream) break;
          if (count == 0) dataSpectrum.XValue.push_back(xEle);
        }

				// read data line by line
        string dataString;
        while (getline(inputFile, dataString)){
          istringstream dataStream(dataString);
          std::vector<float> dataValue;
          string tStamp;
          string dataName;
          dataStream>>tStamp>>dataName;
          while (dataStream) {
            float dataEle = 0;
            dataStream >> dataEle;
            if (!dataStream) break;
            dataValue.push_back(dataEle);
          }
          dataSpectrum.dataSets.push_back(dataValue);
        }
      }
      else continue;
    }
		

    int dataSize = dataSpectrum.dataSets.size();
    cout<<"There are "<<dataSize<<" sets of data."<<endl;

    // Draw the spectrum.
    string dataName = inputFileName;
    size_t txtpos = inputFileName.find(".txt");
    size_t prepos = inputFileName.find("Panel");
    dataName.replace(txtpos, 4, "_");
    dataName.erase(0,prepos);
    for (int i = 0; i < dataSize; i++){
      float* xArray = &dataSpectrum.XValue[0];
      float* dataArray = &dataSpectrum.dataSets.at(i)[0];

      TGraph* sGraphs = new TGraph(1000, xArray, dataArray);
      ostringstream oss;
      oss<<dataName<<i;
      string graphName(oss.str());
      sGraphs->SetName(graphName.c_str());
      //sGraphs->Draw();
      //sGraphs->GetXaxis()->SetRangeUser(350,550);
      sGraphs->Write();
    }
		count +=1;
  }
		dataSpectrum.AssignSpectrum();
		dataSpectrum.CalcMean();
		dataSpectrum.CalcSTDDEV();
		float* xArray = &dataSpectrum.XValue[0];
		float* dataArray = &dataSpectrum.YValue[0];
		float* dataError = &dataSpectrum.YSTDDEV[0];
		TGraphErrors* finalGraph = new TGraphErrors(1000, xArray, dataArray, 0, dataError);
		finalGraph->Write("finalgraph");

  gOutputFile->Write();
  gOutputFile->Close();
	
	TCanvas* tc = new TCanvas("tc", "");
	tc->cd();
	  TPad* corner_pad = new TPad("corner_pad02", "corner_pad",0.0,0.0,0.95,0.95);
  corner_pad->Draw();
  gStyle->SetOptStat(0);
  corner_pad->SetLeftMargin(0.15);
  corner_pad->SetBottomMargin(0.15);
  corner_pad->SetRightMargin(0.02);
  corner_pad->SetTopMargin(0.02);
  corner_pad->cd();
	finalGraph->SetTitle("");
	finalGraph->GetXaxis()->SetRangeUser(350, 500);
	finalGraph->GetXaxis()->SetTitle("Wavelength (nm)");
	finalGraph->GetYaxis()->SetTitle("Relative reflectance (%)");
    finalGraph->GetXaxis()->SetTitleFont(43);
    finalGraph->GetXaxis()->SetTitleSize(30);
    finalGraph->GetXaxis()->SetLabelFont(43);
    finalGraph->GetXaxis()->SetLabelSize(25);
    finalGraph->GetYaxis()->SetTitleFont(43);
    finalGraph->GetYaxis()->SetTitleSize(30);
    finalGraph->GetYaxis()->SetLabelFont(43);
    finalGraph->GetYaxis()->SetLabelSize(25);
	finalGraph->SetLineColor(kAzure);
	finalGraph->SetLineWidth(3);
	finalGraph->SetFillColor(kAzure-4);
	finalGraph->SetFillStyle(3001);
	finalGraph->Draw("APLE3");
	tc->SaveAs(OutputFigure.c_str());
	
  return 0;
}
