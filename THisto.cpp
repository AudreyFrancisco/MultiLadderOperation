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
void THisto::Incr (unsigned int i)
{
    if (i<fDim[0]) {
        if (fSize == 1) ((unsigned char **)fHisto)[0][i]++;
        if (fSize == 2) ((unsigned short int **)fHisto)[0][i]++;
        if (fSize == 4) ((float **)fHisto)[0][i]++;
        if (fSize == 8) ((double **)fHisto)[0][i]++;
    }
}

//___________________________________________________________________
void THisto::Incr (unsigned int i, unsigned int j)
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


//================================================================================
//
//                         TScanHisto
//
//================================================================================


//___________________________________________________________________
TScanHisto::~TScanHisto()
{
    std::map<int, std::shared_ptr<THisto>>::iterator it;
    for (it = fHistos.begin(); it != fHistos.end(); ++it) {
        ((*it).second).reset();
    }
}

//___________________________________________________________________
TScanHisto::TScanHisto( const TScanHisto &sh )
{
    std::map<int, std::shared_ptr<THisto>>::const_iterator it;
    for (it = sh.fHistos.begin(); it != sh.fHistos.end(); ++it) {
        fHistos.insert(*it);
    }
}

//___________________________________________________________________
void TScanHisto::AddHisto( TChipIndex index, std::shared_ptr<THisto> histo )
{
    int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId);
    fHistos.insert (std::pair<int, std::shared_ptr<THisto>>(int_index, histo));
}

//___________________________________________________________________
void TScanHisto::Clear()
{
    std::map<int, std::shared_ptr<THisto>>::iterator it;
    for (it = fHistos.begin(); it != fHistos.end(); ++it) {
        ((*it).second)->Clear();
    }
}


// TODO: (for all) check index for validity
//___________________________________________________________________
void TScanHisto::Incr( TChipIndex index, unsigned int i, unsigned int j )
{
    int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId);
    (fHistos.at(int_index))->Incr(i,j);
}


// TODO: clean up, write missing operator (1-d)
//___________________________________________________________________
double TScanHisto::operator() (TChipIndex index, unsigned int i, unsigned int j) const
{
    int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId);
    return ((*(fHistos.at(int_index)))(i,j));
}
