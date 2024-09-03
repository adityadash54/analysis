#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
using namespace std;

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TKey.h>
#include <TObject.h>
#include <THStack.h>

vector<string> read_list(string input_txt_file_directory)
{
    string list_buffer;
    ifstream data_list;
	data_list.open((input_txt_file_directory).c_str());

    vector<string> output_vec; output_vec.clear();

	while (1)
	{
		data_list >> list_buffer;
		if (!data_list.good())
		{
			break;
		}
		output_vec.push_back(list_buffer);
	}
	cout<<"size : "<<output_vec.size()<<endl;

    return output_vec;
}

void TH1F_fake_add(TH1F * hist_base, TH1F * hist_new)
{
    if (
        hist_base -> GetNbinsX() != hist_new -> GetNbinsX() || 
        hist_base -> GetXaxis() -> GetXmin() != hist_new -> GetXaxis() -> GetXmin() || 
        hist_base -> GetXaxis() -> GetXmax() != hist_new -> GetXaxis() -> GetXmax())
    {
        cout<<"TH1F_fake_add : the two histograms have different bin numbers!"<<endl;
        return;
    }

    for (int i = 1; i < hist_base -> GetNbinsX() + 1; i++)
    {
        hist_base -> SetBinContent(i, hist_base -> GetBinContent(i) + hist_new -> GetBinContent(i));
    }
}

void TH2F_fake_add(TH2F * hist_base, TH2F * hist_new)
{
    if (
        hist_base -> GetNbinsX() != hist_new -> GetNbinsX() || 
        hist_base -> GetXaxis() -> GetXmin() != hist_new -> GetXaxis() -> GetXmin() || 
        hist_base -> GetXaxis() -> GetXmax() != hist_new -> GetXaxis() -> GetXmax() || 
        hist_base -> GetNbinsY() != hist_new -> GetNbinsY() || 
        hist_base -> GetYaxis() -> GetXmin() != hist_new -> GetYaxis() -> GetXmin() || 
        hist_base -> GetYaxis() -> GetXmax() != hist_new -> GetYaxis() -> GetXmax())
    {
        cout<<"TH2F_fake_add : the two histograms have different bin numbers!"<<endl;
        return;
    }

    for (int i = 1; i < hist_base -> GetNbinsX() + 1; i++)
    {
        for (int j = 1; j < hist_base -> GetNbinsY() + 1; j++)
        {
            hist_base -> SetBinContent(i, j, hist_base -> GetBinContent(i, j) + hist_new -> GetBinContent(i, j));
        }
    }
}


int main(int argc, char *argv[])
{

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_directory>" "<file_list_name>" "<output_file_name>" << std::endl;
        return 1; // Exit the program with an error code
    }

    string input_directory  = argv[1];
    string input_file_list  = argv[2];
    string output_file_name = argv[3];

    // string input_directory = "/sphenix/user/ChengWei/sPH_dNdeta/HIJING_ana398_xvtx-0p04cm_yvtx0p24cm_zvtx-20cm_dummyAlignParams/TrackletReco_condor";
    // string input_file_list = "file_list.txt";
    string output_directory = input_directory + "/merged_file_folder";
    // string output_file_name = "merged_hist.root";

    if(std::filesystem::exists(Form("%s",output_directory.c_str())) == false){
        system(Form("mkdir %s", output_directory.c_str()));
        cout<<"----------- check folder exists -----------"<<endl;
        system(Form("ls %s", output_directory.c_str()));
    }
    else 
    {
        cout<<"----------- folder exists already -----------"<<endl;
        system(Form("ls %s", output_directory.c_str()));
    }

    vector<string> file_list = read_list((input_directory + "/" + input_file_list).c_str());
    map<string, TFile*> TFile_in_map; TFile_in_map.clear();
    map<string, TH1F*> TH1F_map; TH1F_map.clear();
    map<string, TH2F*> TH2F_map; TH2F_map.clear();

    int file_counting = 0;

    for (auto file_name : file_list)
    {
        cout<<"running the file : "<<file_name.substr(input_directory.size(),file_name.size())<<endl;
        TFile_in_map[file_name.c_str()] = TFile::Open(Form("%s", file_name.c_str()));

        for (TObject* keyAsObj : *TFile_in_map[file_name.c_str()]->GetListOfKeys())
        {
            
            auto key = dynamic_cast<TKey*>(keyAsObj);
            string hist_name  = key->GetName();
            string class_name = key->GetClassName();

            if (class_name == "TH1F")
            {
                if (TH1F_map.find(hist_name) == TH1F_map.end())
                { 
                    string hist_1D_X_title = ((TH1F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetXaxis()->GetTitle();
                    string hist_1D_Y_title = ((TH1F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetYaxis()->GetTitle();
                    TH1F_map[hist_name.c_str()] = new TH1F(
                        (hist_name + "_merge").c_str(), (hist_name + "_merge;" + hist_1D_X_title + ";" + hist_1D_Y_title).c_str(),
                        ((TH1F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetNbinsX(),
                        ((TH1F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetXaxis()->GetXmin(),
                        ((TH1F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetXaxis()->GetXmax()
                    );
                    
                    TH1F_fake_add(TH1F_map[hist_name.c_str()], ((TH1F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() )));
                }
                else
                { 
                    TH1F_fake_add(TH1F_map[hist_name.c_str()], ((TH1F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))); 
                }
            } 
            else if (class_name == "TH2F")
            {
                if (TH2F_map.find(hist_name) == TH2F_map.end())
                { 
                    string hist_2D_X_title = ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetXaxis()->GetTitle();
                    string hist_2D_Y_title = ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetYaxis()->GetTitle();
                    TH2F_map[hist_name.c_str()] = new TH2F(
                        (hist_name + "_merge").c_str(), (hist_name + "_merge;" + hist_2D_X_title + ";" + hist_2D_Y_title).c_str(),
                        ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetNbinsX(),
                        ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetXaxis()->GetXmin(),
                        ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetXaxis()->GetXmax(),
                        ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetNbinsY(),
                        ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetYaxis()->GetXmin(),
                        ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() ))->GetYaxis()->GetXmax()
                    );

                    TH2F_fake_add(TH2F_map[key->GetName()], ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() )) );
                }
                else
                { 
                    TH2F_fake_add(TH2F_map[key->GetName()], ((TH2F *) TFile_in_map[file_name.c_str()] -> Get( hist_name.c_str() )) ); 
                }
            }
            else 
            {
                cout<<"weird : "<<key->GetClassName()<<endl;
            }

        }

        cout<<"test : "<<((TH2F*) TFile_in_map[file_name.c_str()] -> Get("Eta_Z_reference")) -> GetBinContent(18,8)<<endl;

        // file_counting += 1;
        // if (file_counting == 2) {break;}
    }
    

    TFile * file_out = new TFile(Form("%s/%s", output_directory.c_str(), output_file_name.c_str()), "RECREATE");
    file_out -> cd();
    for (pair<string,TH1F*> hist : TH1F_map)
    {
        hist.second -> Write((hist.first).c_str(), TObject::kOverwrite);
    }

    for (pair<string,TH2F*> hist : TH2F_map)
    {
        hist.second -> Write((hist.first).c_str(), TObject::kOverwrite);
    }
    file_out -> Close();

    cout<<"the merged file is closed"<<endl;

    // for (pair<string, TFile*> file : TFile_in_map)
    // {
    //     file.second -> Close();
    //     cout<<"file : "<<file.first<<" is closed"<<endl;
    // }

    return 0;
}
