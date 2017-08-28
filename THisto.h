#ifndef THISTO_H
#define THISTO_H

#include <string>
#include <map>
#include <memory>

typedef struct
{
    unsigned int boardIndex;
    unsigned int dataReceiver;
    unsigned int chipId;
} TChipIndex;

class THisto {
    
private:
    int           fNdim;      ///< Number of dimensions (1 or 2)
    std::string   fName;      ///< Histogram name
    std::string   fTitle;     ///< Histogram title
    unsigned int  fDim[2];    ///< Dimensions
    double        fLim[2][2]; ///< Limits
    void**        fHisto;     ///< Histogram
    unsigned int  fSize;      ///< Word size
    double        fTrash;     ///< Trash bin
    
public:
    /// Default constructor ("0-Dim histogram")
    THisto();
    /// Constructor 1D
    THisto( std::string name, std::string title,
           unsigned int nbin, double xmin, double xmax );
    /// Constructor 2D
    THisto( std::string name, std::string title,
           unsigned int nbin1, double xmin1, double xmax1,
            unsigned int nbin2, double xmin2, double xmax2 );
    /// Constructor 2D
    THisto( std::string name, std::string title,
            unsigned int size, unsigned int nbin1, double xmin1, double xmax1,
            unsigned int nbin2, double xmin2, double xmax2 );
    /// Copy constructor
    THisto (const THisto &h);
    /// Destructor
    ~THisto();
    /// Assignment operator
    THisto &operator=  (const THisto &h);
    /// Bin read access 1d
    double operator()  (unsigned int i) const;
    /// Bin read access 2d
    double operator()  (unsigned int i, unsigned int j) const;
    /// Bin write access 1d
    void   Set         (unsigned int i, double val);
    /// Bin write access 2d
    void   Set         (unsigned int i, unsigned int j, double val);
    void   Incr        (unsigned int i);
    void   Incr        (unsigned int i, unsigned int j);
    /// Reset histo - NO MEMORY DISCARD
    void   Clear       ();
    
    /// Getter methods
    std::string GetName ()      const { return fName; };
    std::string GetTitle()      const { return fTitle; };
    int         GetNDim ()      const { return fNdim; };
    int         GetNBin (int d) const {
        if (d >=0 && d <= 1) return fDim[d]; else return 0; }
    double      GetMin  (int d) const {
        if (d >=0 && d <= 1) return fLim[d][0]; else return 0; }
    double      GetMax  (int d) const {
        if (d >=0 && d <= 1) return fLim[d][1]; else return 0; }
};

class TScanHisto {
private:
    std::map<int, std::shared_ptr<THisto>> fHistos;
public:
    /// Default constructor
    TScanHisto() {};
    /// Default destructor
    ~TScanHisto();
    /// Copy constructor;
    TScanHisto(const TScanHisto &sh);
    /// Bin read access 2d
    double operator()  (TChipIndex index, unsigned int i, unsigned int j) const;
    
    void AddHisto( TChipIndex index, std::shared_ptr<THisto> histo );
    int  GetSize() {return fHistos.size();}
    void Clear();
    void Incr( TChipIndex index, unsigned int i, unsigned int j );
};

#endif
