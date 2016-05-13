#include "ATFInterface.h"

#include <stdio.h>
#include <stdlib.h>

ATFInterface* ATFInterface::S_CURRENT = NULL;

ATFInterface::ATFInterface()
: m_menu(NULL)
, m_option_designer(NULL)
{
}

ATFInterface* ATFInterface::Instance()
{
  if (!S_CURRENT)
    S_CURRENT = new ATFInterface();

  return S_CURRENT;
}

void ATFInterface::Build()
{
  m_option_designer = IupItem("Alterar Função", NULL);
  IupSetAttribute(m_option_designer, "ACTIVE", "YES");

  IupSetCallback(m_option_designer, "ACTION", (Icallback)ATFInterface::OpenDesignerDialog);

  m_menu = IupSubmenu("ATFG", IupMenu(m_option_designer, NULL));
}

int ATFInterface::OpenDesignerDialog(Ihandle* ih)
{
  ATFInterface* current = ATFInterface::Instance();

  Ihandle* bar = IupVal("VERTICAL");
  IupSetAttribute(bar, "ACTIVE", "YES");
  IupSetCallback(bar, "VALUECHANGED_CB", (Icallback)ATFInterface::Value);
  
  Ihandle *dlg = IupDialog(IupHbox(bar, NULL));
  IupSetAttribute(dlg, "TITLE", "Function Designer");

  IupPopup(dlg, IUP_CURRENT, IUP_CURRENT);
  return IUP_DEFAULT;
}

int ATFInterface::Value(Ihandle* ih)
{
  char *val = IupGetAttribute(ih, "VALUE");
  printf("Changed: %s\n", val);
  return IUP_DEFAULT;
}