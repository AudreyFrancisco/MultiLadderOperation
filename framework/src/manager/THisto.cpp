#include "THisto.h"
#include <iostream>

//___________________________________________________________________
THisto::THisto()
{
    fNdim = 0;
    fName = "";
    fTitle = "";
    fDim[0] = 0;
    fDim[1] = 0;
    fLim[0][0] = 0;
    fLim[0][1] = 0;
    fLim[1][0] = 0;
    fLim[1][1] = 0;
    fHisto = 0;
    fTrash = 0;
    fSize = 8;
}

//___________________________________________________________________
THisto::THisto( std::string name, std::string title,
                unsigned int nbin, double xmin, double xmax ) {
    fNdim = 1;
    fName = name;
    fTitle = title;
    fDim[0] = nbin;
    fDim[1] = 1;
    fLim[0][0] = xmin;
    fLim[0][1] = xmax;
    fLim[1][0] = 0;
    fLim[1][1] = 0;
    fHisto = new void*[1];
    fHisto[0] = (void *)new double[fDim[0]];
    for (unsigned int i=0; i<fDim[0]; i++) ((double **)fHisto)[0][i] = 0;
    fTrash = 0;
    fSize = 8;
}

//___________________________________________________________________
THisto::THisto( std::string name, std::string title,
                unsigned int nbin1, double xmin1, double xmax1,
                unsigned int nbin2, double xmin2, double xmax2 )
{
    fNdim = 2;
    fName = name;
    fTitle = title;
    fDim[0] = nbin1;
    fDim[1] = nbin2;
    fLim[0][0] = xmin1;
    fLim[0][1] = xmax1;
    fLim[1][0] = xmin2;
    fLim[1][1] = xmax2;
    fHisto = new void*[fDim[1]];
    for (unsigned int j=0; j<fDim[1]; j++) {
        fHisto[j] = (void *)new double[fDim[0]];
        for (unsigned int i=0; i<fDim[0]; i++) ((double **)fHisto)[j][i] = 0;
    }
    fTrash = 0;
    fSize = 8;
}

//___________________________________________________________________
THisto::THisto( std::string name, std::string title,
                unsigned int size, unsigned int nbin1, double xmin1, double xmax1,
                unsigned int nbin2, double xmin2, double xmax2 )
{
    fNdim = 2;
    if (size != 1 && size != 2 && size != 4) size = 8;
    fSize = size;
    fName = name;
    fTitle = title;
    fDim[0] = nbin1;
    fDim[1] = nbin2;
    fLim[0][0] = xmin1;
    fLim[0][1] = xmax1;
    fLim[1][0] = xmin2;
    fLim[1][1] = xmax2;
    fHisto = new void*[fDim[1]];
    if (fSize == 1) {
        for (unsigned int j=0; j<fDim[1]; j++) {
            fHisto[j] = (void *)new unsigned char[fDim[0]];
            for (unsigned int i=0; i<fDim[0]; i++) ((unsigned char **)fHisto)[j][i] = 0;
        }
    } else if (fSize == 2) {
        for (unsigned int j=0; j<fDim[1]; j++) {
            fHisto[j] = (void *)new unsigned short int[fDim[0]];
            for (unsigned int i=0; i<fDim[0]; i++) ((unsigned short int **)fHisto)[j][i] = 0;
        }
    } else if (fSize == 4) {
        for (unsigned int j=0; j<fDim[1]; j++) {
            fHisto[j] = (void *)new float[fDim[0]];
            for (unsigned int i=0; i<fDim[0]; i++) ((float **)fHisto)[j][i] = 0;
        }
    } else {
        for (unsigned int j=0; j<fDim[1]; j++) {
            fHisto[j] = (void *)new double[fDim[0]];
            for (unsigned int i=0; i<fDim[0]; i++) ((double **)fHisto)[j][i] = 0;
        }
    }
    fTrash = 0;
}

//___________________________________________________________________
THisto::THisto( const THisto &h )
{
    fNdim = h.fNdim;
    fName = h.fName;
    fTitle = h.fTitle;
    fDim[0] = h.fDim[0];
    fDim[1] = h.fDim[1];
    fLim[0][0] = h.fLim[0][0];
    fLim[0][1] = h.fLim[0][1];
    fLim[1][0] = h.fLim[1][0];
    fLim[1][1] = h.fLim[1][1];
    fSize = h.fSize;
    fHisto = new void*[fDim[1]];
    if (fSize == 1) {
        for (unsigned int j=0; j<fDim[1]; j++) {
            fHisto[j] = (void *)new unsigned char[fDim[0]];
            for (unsigned int i=0; i<fDim[0]; i++) ((unsigned char **)fHisto)[j][i] = ((unsigned char **)h.fHisto)[j][i];
        }
    } else if (fSize == 2) {
        for (unsigned int j=0; j<fDim[1]; j++) {
            fHisto[j] = (void *)new unsigned short int[fDim[0]];
            for (unsigned int i=0; i<fDim[0]; i++) ((unsigned short int **)fHisto)[j][i] = ((unsigned short int **)h.fHisto)[j][i];
        }
    } else if (fSize == 4) {
        for (unsigned int j=0; j<fDim[1]; j++) {
            fHisto[j] = (void *)new float[fDim[0]];
            for (unsigned int i=0; i<fDim[0]; i++) ((float **)fHisto)[j][i] = ((float **)h.fHisto)[j][i];
        }
    } else {
        for (unsigned int j=0; j<fDim[1]; j++) {
            fHisto[j] = (void *)new double[fDim[0]];
            for (unsigned int i=0; i<fDim[0]; i++) ((double **)fHisto)[j][i] = ((double **)h.fHisto)[j][i];
        }
    }
    fTrash = 0;
}

//___________________________________________________________________
THisto::~THisto()
{
    for (unsigned int j=0; j<fDim[1]; j++) {
        if (fSize == 1) delete[] ((unsigned char **)fHisto)[j];
        if (fSize == 2) delete[] ((unsigned short int **)fHisto)[j];
        if (fSize == 4) delete[] ((float **)fHisto)[j];
        if (fSize == 8) delete[] (double *)(fHisto[j]);
    }
    delete[] fHisto;
}

//___________________________________________________________________
THisto &THisto::operator=( const THisto &h )
{
    if (&h == this) {
        return *this;
    } else {
        unsigned int j;
        for (j=0; j<fDim[1]; j++) {
            if (fSize == 1) delete[] ((unsigned char **)fHisto)[j];
            if (fSize == 2) delete[] ((unsigned short int **)fHisto)[j];
            if (fSize == 4) delete[] ((float **)fHisto)[j];
            if (fSize == 8) delete[] ((double **)fHisto)[j];
        }
        delete[] fHisto;
        fNdim = h.fNdim;
        fName = h.fName;
        fTitle = h.fTitle;
        fDim[0] = h.fDim[0];
        fDim[1] = h.fDim[1];
        fLim[0][0] = h.fLim[0][0];
        fLim[0][1] = h.fLim[0][1];
        fLim[1][0] = h.fLim[1][0];
        fLim[1][1] = h.fLim[1][1];
        fSize = h.fSize;
        fHisto = new void*[fDim[1]];
        if (fSize == 1) {
            for (unsigned int j=0; j<fDim[1]; j++) {
                fHisto[j] = (void *)new unsigned char[fDim[0]];
                for (unsigned int i=0; i<fDim[0]; i++) ((unsigned char **)fHisto)[j][i] = ((unsigned char **)h.fHisto)[j][i];
            }
        } else if (fSize == 2) {
            for (unsigned int j=0; j<fDim[1]; j++) {
                fHisto[j] = (void *)new unsigned short int[fDim[0]];
                for (unsigned int i=0; i<fDim[0]; i++) ((unsigned short int **)fHisto)[j][i] = ((unsigned short int **)h.fHisto)[j][i];
            }
        } else if (fSize == 4) {
            for (unsigned int j=0; j<fDim[1]; j++) {
                fHisto[j] = (void *)new float[fDim[0]];
                for (unsigned int i=0; i<fDim[0]; i++) ((float **)fHisto)[j][i] = ((float **)h.fHisto)[j][i];
            }
        } else {
            for (unsigned int j=0; j<fDim[1]; j++) {
                fHisto[j] = (void *)new double[fDim[0]];
                for (unsigned int i=0; i<fDim[0]; i++) ((double **)fHisto)[j][i] = ((double **)h.fHisto)[j][i];
            }
        }
        fTrash = 0;
        return *this;
    }
}

//___________________________________________________________________
double THisto::operator()(unsigned int i) const
{
    if (i<fDim[0]) {
        if (fSize == 1) return (double)(((unsigned char **)fHisto)[0][i]);
        if (fSize == 2) return (double)(((unsigned short int **)fHisto)[0][i]);
        if (fSize == 4) return (double)(((float **)fHisto)[0][i]);
        if (fSize == 8) return (double)(((double **)fHisto)[0][i]);
    }
    return fTrash;
}

//___________________________________________________________________
double THisto::operator()(unsigned int i, unsigned int j) const
{
    if (i<fDim[0] && j<fDim[1]) {
        if (fSize == 1) return (double)(((unsigned char **)fHisto)[j][i]);
        if (fSize == 2) return (double)(((unsigned short int **)fHisto)[j][i]);
        if (fSize == 4) return (double)(((float **)fHisto)[j][i]);
        if (fSize == 8) return (double)(((double **)fHisto)[j][i]);
    }
    return fTrash;
}

//___________________________________________________________________
void THisto::Set(unsigned int i, double val)
{
    if (i<fDim[0]) {
        if (fSize == 1) ((unsigned char **)fHisto)[0][i] = (unsigned char)val;
        if (fSize == 2) ((unsigned short int **)fHisto)[0][i] = (unsigned short int)val;
        if (fSize == 4) ((float **)fHisto)[0][i] = (float)val;
        if (fSize == 8) ((double **)fHisto)[0][i] = val;
    }
}

//___________________________________________________________________
void THisto::Set(unsigned int i, unsigned int j, double val)
{
    if (i<fDim[0] && j<fDim[1]) {
        if (fSize == 1) ((unsigned char **)fHisto)[j][i] = (unsigned char)val;
        if (fSize == 2) ((unsigned short int **)fHisto)[j][i] = (unsigned short int)val;
        if (fSize == 4) ((float **)fHisto)[j][i] = (float)val;
        if (fSize == 8) ((double **)fHisto)[j][i] = val;
    }
}

//___________________________________________________________________
void THisto::Incr(unsigned int i)
{
    if (i<fDim[0]) {
        if (fSize == 1) ((unsigned char **)fHisto)[0][i]++;
        if (fSize == 2) ((unsigned short int **)fHisto)[0][i]++;
        if (fSize == 4) ((float **)fHisto)[0][i]++;
        if (fSize == 8) ((double **)fHisto)[0][i]++;
    }
}

//___________________________________________________________________
void THisto::Incr(unsigned int i, unsigned int j)
{
    if (i<fDim[0] && j<fDim[1]) {
        if (fSize == 1) ((unsigned char **)fHisto)[j][i]++;
        if (fSize == 2) ((unsigned short int **)fHisto)[j][i]++;
        if (fSize == 4) ((float **)fHisto)[j][i]++;
        if (fSize == 8) ((double **)fHisto)[j][i]++;
    }
}

//___________________________________________________________________
void THisto::Clear()
{
    for (unsigned int j=0; j<fDim[1]; j++) {
        for (unsigned int i=0; i<fDim[0]; i++) {
            if (fSize == 1) ((unsigned char **)fHisto)[j][i] = 0;
            if (fSize == 2) ((unsigned short int **)fHisto)[j][i] = 0;
            if (fSize == 4) ((float **)fHisto)[j][i] = 0;
            if (fSize == 8) ((double **)fHisto)[j][i] = 0;
        }
    }
    fTrash = 0;
}

//___________________________________________________________________
unsigned int THisto::GetNEntries() const
{
    double nEntries = 0;
    for (unsigned int j=0; j<fDim[1]; j++) {
        for (unsigned int i=0; i<fDim[0]; i++) {
            if (fSize == 1) nEntries += ((unsigned char **)fHisto)[j][i];
            if (fSize == 2) nEntries += ((unsigned short int **)fHisto)[j][i];
            if (fSize == 4) nEntries += ((float **)fHisto)[j][i];
            if (fSize == 8) nEntries += ((double **)fHisto)[j][i];
        }
    }
    if ( nEntries < 0 ) {
        nEntries = 0;
    }
    return (unsigned int)nEntries;
}


//================================================================================
//
//                         TScanHisto
//
//================================================================================

#pragma mark - constructors / destructor

//___________________________________________________________________
TScanHisto::TScanHisto() :
fIndex( -1 )
{
    
}

//___________________________________________________________________
TScanHisto::TScanHisto( const TScanHisto &sh )
{
    std::map<int, THisto>::const_iterator it;
    for (it = sh.fHistos.begin(); it != sh.fHistos.end(); ++it) {
        fHistos.insert(*it);
    }
    SetIndex(sh.GetIndex());
    for (std::vector<common::TChipIndex>::iterator it = fChipList.begin(); it != fChipList.end(); ++it) {
        fChipList.push_back(*it);
    }
}

//___________________________________________________________________
TScanHisto::~TScanHisto()
{
    fHistos.clear();
}

#pragma mark - getters

// TODO: clean up
//___________________________________________________________________
double TScanHisto::operator() ( common::TChipIndex index, unsigned int i, unsigned int j ) const
{
    if ( GetVerboseLevel() > kULTRACHATTY ) {
        std::cout << "TScanHisto::operator(idx, i, j) - B " << std::dec << index.boardIndex
        << " Rx " << index.dataReceiver << " chipId " << index.chipId << std::endl;
    }
    int int_index =  (index.ladderId << 12)
        | (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf );
    return (fHistos.at(int_index))(i,j);
}

//TODO: clean up
//___________________________________________________________________
double TScanHisto::operator() ( common::TChipIndex index, unsigned int i ) const
{
    if ( GetVerboseLevel() > kULTRACHATTY ) {
        std::cout << "TScanHisto::operator(idx, i) - B " << std::dec << index.boardIndex
        << " Rx " << index.dataReceiver << " chipId " << index.chipId << std::endl;
    }
    int int_index =  (index.ladderId << 12)
        | (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
    return (fHistos.at(int_index))(i);
}

//___________________________________________________________________
common::TChipIndex TScanHisto::GetChipIndex( const unsigned int i ) const
{
    return fChipList.at(i);
}

//___________________________________________________________________
unsigned int TScanHisto::GetChipNEntries( common::TChipIndex index ) const
{
    if ( GetVerboseLevel() > kULTRACHATTY ) {
        std::cout << "TScanHisto::GetChipNEntries() - B " << std::dec << index.boardIndex
        << " Rx " << index.dataReceiver << " chipId " << index.chipId << std::endl;
    }
    int int_index =  (index.ladderId << 12)
        | (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
    return (fHistos.at(int_index)).GetNEntries();
}

#pragma mark - other

//___________________________________________________________________
void TScanHisto::AddHisto( common::TChipIndex index, THisto histo )
{
    if ( GetVerboseLevel() > kULTRACHATTY ) {
        std::cout << "TScanHisto::AddHisto() - B " << std::dec << index.boardIndex
        << " Rx " << index.dataReceiver << " chipId " << index.chipId << std::endl;
    }
    int int_index =  (index.ladderId << 12)
        | (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf );
    fHistos.insert (std::pair<int, THisto>(int_index, histo));
}


// TODO: (for all) check index for validity
//___________________________________________________________________
void TScanHisto::Incr( common::TChipIndex index, unsigned int i, unsigned int j )
{
    if ( GetVerboseLevel() > kULTRACHATTY ) {
        std::cout << "TScanHisto::Incr() - B " << std::dec << index.boardIndex
        << " Rx " << index.dataReceiver << " chipId " << index.chipId << std::endl;
    }
   int int_index =  (index.ladderId << 12)
        | (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
    (fHistos.at(int_index)).Incr(i,j);
}

//___________________________________________________________________
void TScanHisto::Incr( common::TChipIndex index, unsigned int i )
{
    if ( GetVerboseLevel() > kULTRACHATTY ) {
        std::cout << "TScanHisto::Incr() - B " << std::dec << index.boardIndex
        << " Rx " << index.dataReceiver << " chipId " << index.chipId << std::endl;
    }
   int int_index = (index.ladderId << 12)
        | (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
    (fHistos.at(int_index)).Incr(i);
}

//___________________________________________________________________
void TScanHisto::FindChipList()
{
    fChipList.clear();
    for (std::map<int, THisto>::iterator it = fHistos.begin(); it != fHistos.end(); ++it) {
        int        intIndex = it->first;
        common::TChipIndex index;
        index.ladderId     = (intIndex >> 12);
        index.boardIndex   = (intIndex >> 8) & 0xf;
        index.dataReceiver = (intIndex >> 4) & 0xf;
        index.chipId       =  intIndex       & 0xf;
        fChipList.push_back(index);
    }
}

//___________________________________________________________________
bool TScanHisto::IsValidChipIndex( const common::TChipIndex idx )
{
    for ( unsigned int i = 0; i < GetChipListSize(); i++ ) {
        if ( common::SameChipIndex( idx, GetChipIndex(i) ) ) {
            return true;
        }
    }
    return false;
}

//___________________________________________________________________
void TScanHisto::Clear()
{
    std::map<int, THisto>::iterator it;
    for (it = fHistos.begin(); it != fHistos.end(); ++it) {
        ((*it).second).Clear();
    }
    fIndex = -1;
    fChipList.clear();
}

