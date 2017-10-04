#ifndef THISTO_H
#define THISTO_H

#include <string>
#include <map>
#include <vector>

#include "Common.h"
#include "TVerbosity.h"

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
    unsigned int GetNEntries() const;
    bool HasData() const;
};

class TScanHisto : public TVerbosity {
private:
    std::map<int, THisto> fHistos;
    int fIndex;
    std::vector<common::TChipIndex> fChipList;
    
public:
    
#pragma mark - constructors / destructor
    
    /// Default constructor
    TScanHisto();
    
    /// Copy constructor
    TScanHisto (const TScanHisto &sh);
    
    /// Default destructor
    ~TScanHisto();
    
#pragma mark - setters
    
    inline void SetIndex    (int aIndex) {fIndex = aIndex;};

#pragma mark - getters

    /// Bin read access 2d
    double operator()  (common::TChipIndex index, unsigned int i, unsigned int j) const;
    
    /// Bin read access 1d
    double operator()  (common::TChipIndex index, unsigned int i) const;
    
    inline int  GetSize() {return fHistos.size();}
    inline int  GetIndex() const     {return fIndex;};
    inline std::map<int,THisto> GetHistoMap() {return fHistos;}
    inline unsigned int GetChipListSize() {return fChipList.size();}
    common::TChipIndex GetChipIndex( const unsigned int i ) const;
    unsigned int GetChipNEntries(common::TChipIndex index) const;
    bool HasData(common::TChipIndex index) const;

#pragma mark - other
    
    void AddHisto( common::TChipIndex index, THisto histo );
    void Incr( common::TChipIndex index, unsigned int i, unsigned int j );
    void Incr        (common::TChipIndex index, unsigned int i);
    void FindChipList();
    bool IsValidChipIndex( const common::TChipIndex idx );
    void Clear();
};

#endif
