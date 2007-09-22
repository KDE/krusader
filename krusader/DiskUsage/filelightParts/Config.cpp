
#include "Config.h"
#include <kconfig.h>
#include <kglobal.h>
#include <ksharedconfig.h>


bool Config::varyLabelFontSizes = true;
bool Config::showSmallFiles = false;
uint Config::contrast = 50;
uint Config::antiAliasFactor = 2;
uint Config::minFontPitch = 10;
uint Config::defaultRingDepth = 4;
Filelight::MapScheme Config::scheme;


inline KConfigGroup
Filelight::Config::kconfig()
{
    KSharedConfigPtr config = KGlobal::config();
    return KConfigGroup( config, "DiskUsage" );
}

void
Filelight::Config::read()
{
    KConfigGroup group = kconfig();

    varyLabelFontSizes = group.readEntry( "varyLabelFontSizes", true );
    showSmallFiles     = group.readEntry( "showSmallFiles", false );
    contrast           = group.readEntry( "contrast", 50 );
    antiAliasFactor    = group.readEntry( "antiAliasFactor", 2 );
    minFontPitch       = group.readEntry( "minFontPitch", QFont().pointSize() - 3);
    scheme = (MapScheme) group.readEntry( "scheme", 0 );

    defaultRingDepth   = 4;
}

void
Filelight::Config::write()
{
    KConfigGroup group = kconfig();

    group.writeEntry( "varyLabelFontSizes", varyLabelFontSizes );
    group.writeEntry( "showSmallFiles", showSmallFiles);
    group.writeEntry( "contrast", contrast );
    group.writeEntry( "antiAliasFactor", antiAliasFactor );
    group.writeEntry( "minFontPitch", minFontPitch );
    group.writeEntry( "scheme", (int)scheme );
}
