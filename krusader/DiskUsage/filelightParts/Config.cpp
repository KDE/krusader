
#include "Config.h"
#include <kconfig.h>
#include <kglobal.h>


bool Config::varyLabelFontSizes = true;
bool Config::showSmallFiles = false;
uint Config::contrast = 50;
uint Config::antiAliasFactor = 2;
uint Config::minFontPitch = 10;
uint Config::defaultRingDepth = 2;
Filelight::MapScheme Config::scheme;


inline KConfig&
Filelight::Config::kconfig()
{
    KConfig *config = KGlobal::config();
    config->setGroup( "DiskUsage" );
    return *config;
}

void
Filelight::Config::read()
{
    const KConfig &config = kconfig();

    varyLabelFontSizes = config.readBoolEntry( "varyLabelFontSizes", true );
    showSmallFiles     = config.readBoolEntry( "showSmallFiles", false );
    contrast           = config.readNumEntry( "contrast", 50 );
    antiAliasFactor    = config.readNumEntry( "antiAliasFactor", 2 );
    minFontPitch       = config.readNumEntry( "minFontPitch", QFont().pointSize() - 3);
    scheme = (MapScheme) config.readNumEntry( "scheme", 0 );

    defaultRingDepth   = 2;
}

void
Filelight::Config::write()
{
    KConfig &config = kconfig();

    config.writeEntry( "varyLabelFontSizes", varyLabelFontSizes );
    config.writeEntry( "showSmallFiles", showSmallFiles);
    config.writeEntry( "contrast", contrast );
    config.writeEntry( "antiAliasFactor", antiAliasFactor );
    config.writeEntry( "minFontPitch", minFontPitch );
    config.writeEntry( "scheme", scheme );
}
