#ifndef IATFGenerator_H
#define IATFGenerator_H

class ITransferFunction;

class IATFGenerator
{
public:
  virtual ITransferFunction* GetTransferFunction() = 0;
};

#endif