#include "AlpideDecoder.h"
#include <stdint.h>
#include <iostream>

//using namespace AlpideDecoder;


TDataType AlpideDecoder::GetDataType(unsigned char dataWord) {
  if      (dataWord == 0xff)          return DT_IDLE;
  else if (dataWord == 0xf1)          return DT_BUSYON;
  else if (dataWord == 0xf0)          return DT_BUSYOFF;
  else if ((dataWord & 0xf0) == 0xa0) return DT_CHIPHEADER;
  else if ((dataWord & 0xf0) == 0xb0) return DT_CHIPTRAILER;
  else if ((dataWord & 0xf0) == 0xe0) return DT_EMPTYFRAME;
  else if ((dataWord & 0xe0) == 0xc0) return DT_REGIONHEADER;
  else if ((dataWord & 0xc0) == 0x40) return DT_DATASHORT;
  else if ((dataWord & 0xc0) == 0x0)  return DT_DATALONG;
  else return DT_UNKNOWN;
}


void AlpideDecoder::DecodeChipHeader (unsigned char *data, int &chipId, unsigned int &bunchCounter) {
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  bunchCounter = data_field & 0xff;
  chipId       = (data_field >> 8) & 0xf;
}


void AlpideDecoder::DecodeChipTrailer (unsigned char *data, int &flags) {
  flags = data[0] & 0xf;
}


void AlpideDecoder::DecodeRegionHeader (unsigned char *data, int &region) {
  region = data[0] & 0x1f;
}


void AlpideDecoder::DecodeEmptyFrame (unsigned char *data, int &chipId, unsigned int &bunchCounter) {
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  bunchCounter = data_field & 0xff;
  chipId       = (data_field >> 8) & 0xf;
}


void AlpideDecoder::DecodeDataWord (unsigned char *data, int region, std::vector <TPixHit> *hits, bool datalong) {
  TPixHit hit;
  int     address, hitmap_length;

  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  hit.region = region;
  hit.dcol   = (data_field & 0x3c00) >> 10;
  address    = (data_field & 0x03ff);

  if (hits->size() > 0) {
    if ((hit.region == hits->back().region) && (hit.dcol == hits->back().dcol) && (address == hits->back().address)) {
      std::cout << "Warning, received pixel " << hit.region << "/" << hit.dcol << "/" << hit.address <<  " twice." << std::endl;
    }
    else if ((hit.region == hits->back().region) && (hit.dcol == hits->back().dcol) && (address < hits->back().address)) {
      std::cout << "Warning, address of pixel " << hit.region << "/" << hit.dcol << "/" << hit.address <<  " is lower than previous one ("<< hits->back().address << ") in same double column." << std::endl;
    }
  }

  if (datalong) {
    hitmap_length = 7;
  }
  else {
    hitmap_length = 0;
  }

  for (int i = -1; i < hitmap_length; i ++) {
    if ((i >= 0) && (! (data[2] >> i) & 0x1)) continue;     
    hit.address = address + (i + 1);
    hits->push_back (hit);
  }
}


bool AlpideDecoder::DecodeEvent (unsigned char *data, int nBytes, std::vector <TPixHit> *hits) {
  int       byte    = 0;
  int       region  = -1;
  int       chip    = 0;
  int       flags   = 0;
  bool      started = false; // event has started, i.e. chip header has been found
  bool      finished = false; // event trailer found
  TDataType type;

  unsigned int BunchCounterTmp;

  while (byte < nBytes) {
    type = GetDataType (data[byte]);
    switch (type) {
    case DT_IDLE:
      byte +=1;
      break;
    case DT_BUSYON:
      byte += 1;
      break;
    case DT_BUSYOFF:
      byte += 1;
      break;
    case DT_EMPTYFRAME:
      started = true;
      DecodeEmptyFrame (data + byte, chip, BunchCounterTmp);
      byte += 2;
      finished = true;
      break;
    case DT_CHIPHEADER:
      started = true;
      DecodeChipHeader (data + byte, chip, BunchCounterTmp);
      byte += 2;
      break;
    case DT_CHIPTRAILER:
      if (!started) {
        std::cout << "Error, chip trailer found before chip header" << std::endl;
        return false; 
      }
      if (finished) {
        std::cout << "Error, chip trailer found after event was finished" << std::endl;
        return false;  
      }
      DecodeChipTrailer (data + byte, flags);
      finished = true;
      byte += 1;
      break;
    case DT_REGIONHEADER:
      if (!started) {
        std::cout << "Error, region header found before chip header or after chip trailer" << std::endl;
        return false;
      }
      DecodeRegionHeader (data + byte, region);
      byte +=1;
      break;
    case DT_DATASHORT:
      if (!started) {
        std::cout << "Error, hit data found before chip header or after chip trailer" << std::endl;
        return false;
      }
      if (hits) {
        DecodeDataWord (data + byte, region, hits, false);
      }
      byte += 2;
      break;
    case DT_DATALONG:
      if (!started) {
        std::cout << "Error, hit data found before chip header or after chip trailer" << std::endl;
        return false;
      }
      if (hits) {
        DecodeDataWord (data + byte, region, hits, true);
      }
      byte += 3;
      break;
    case DT_UNKNOWN:
      std::cout << "Error, data of unknown type 0x" << std::hex << data[byte] << std::dec << std::endl;
      return false;
    }
  }
  //std::cout << "Found " << Hits->size() - NOldHits << " hits" << std::endl;
  if (started && finished) return true;
  else {
    if (started && !finished) {
      std::cout << "Warning, event not finished at end of data" << std::endl;
      return false;
    }
    if (!started) {
      std::cout << "Warning, event not started at end of data" << std::endl;
      return false;
    }
  }
  return true;
}
