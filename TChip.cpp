#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "TChip.h"

using namespace std;

TChip::TChip() :
    fEnabled( true ),
    fChipId( TChipData::kInitValue ),
    fControlInterface( TChipData::kInitValue ),
    fReceiver( TChipData::kInitValue )
{
    
}

TChip::TChip( const int chipId, const int ci, const int rc ) :
    fEnabled( true )
{
    SetChipId( chipId );
    SetControlInterface( ci );
    SetReceiver( rc );
}

void TChip::SetChipId( const int chipId )
{
    if ( chipId <= TChipData::kInitValue ) {
        throw TChipError( "TChip::SetChipId() - wrong initialization" );
    }
    fChipId = chipId;
}

void TChip::SetControlInterface( const int ci )
{
    if ( ci <= TChipData::kInitValue ) {
        throw TChipError( "TChip::SetControlInterface() - wrong initialization" );
    }
    fControlInterface = ci;
}

void TChip::SetReceiver( const int rc )
{
    if ( rc <= TChipData::kInitValue ) {
        throw TChipError( "TChip::SetReceiver() - wrong initialization" );
    }
    fReceiver = rc;
}

int TChip::GetChipId() const
{
    if ( fChipId <= TChipData::kInitValue ) {
        cout << " WARNING > TChip::GetChipId() - return a non-initialized value" << endl;
    }
    return fChipId;
}

int TChip::GetControlInterface() const
{
    if ( fControlInterface <= TChipData::kInitValue ) {
        cout << " WARNING > TChip::GetControlInterface() - return a non-initialized value" << endl;
    }
    return fControlInterface;
}

int TChip::GetReceiver() const
{
    if ( fReceiver <= TChipData::kInitValue ) {
        cout << " WARNING > TChip::GetReceiver() - return a non-initialized value" << endl;
    }
    return fReceiver;
}


TChipError::TChipError( const string& arg )
{
    msg = "ERROR > " + arg;
}
