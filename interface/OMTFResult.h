#ifndef OMTF_OMTFResult_H
#define OMTF_OMTFResult_H

#include <vector>
#include <ostream>

class OMTFResult{

 public:

  typedef std::vector<unsigned int> vector1D;
  typedef std::vector<vector1D> vector2D;

  OMTFResult();

  const OMTFResult::vector2D & getResults() const {return results;}

  const OMTFResult::vector1D & getSummaryVals() const {return results1D;}

  const OMTFResult::vector1D & getSummaryHits() const {return hits1D;}

  const OMTFResult::vector1D & getRefPhis() const {return refPhi1D;}


  void addResult(unsigned int iRefLayer,
		 unsigned int iLayer,
		 unsigned int val,
		 int iRefPhi);

  void finalise();

  void clear();

  friend std::ostream & operator << (std::ostream &out, const OMTFResult & aResult);

 private:

  ///Pdf weight found for each layer
  ///First index: layer number
  ///Second index: ref layer number
  vector2D results; 

  ///Reference phi for each reference layer
  vector1D refPhi1D; 

  ///Sum of pdf weights for each reference layer
  vector1D results1D; 

  ///Number of hits for each reference layer
  vector1D hits1D; 

};


#endif
