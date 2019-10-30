eval 'exec perl -S $0 ${1+"$@"}'  # -*- Mode: perl -*-
    if $running_under_some_shell; # makeIocCdCommands.pl

use Cwd;

$cwd  = cwd();
#hack for sun4
$cwd =~ s|/tmp_mnt||;
$arch = $ARGV[0];

unlink("cdCommands");
open(OUT,">cdCommands") or die "$! opening cdCommands";

$top = $cwd;
$top =~ s/iocBoot.*//;
$appbin = $top . "bin/${arch}";
print OUT "apptop = \"$top\"\n";
print OUT "startup = \"$cwd\"\n";
print OUT "appbin = \"$appbin\"\n";
#look for SHARE in <top>/config/RELEASE
$release = $cwd;
$release =~ s/iocBoot.*//;
$release = $release . "config/RELEASE";
if (-r "$release") {
    open(IN, "$release") or die "Cannot open $release\n";
    while (<IN>) {
	chomp;
	s/^SHARE\s*=\s*// and $share = $_;
	s/^MASTER_ARCH\s*=\s*// and $master_arch = $_;

    }
    close IN;
}
if ( "$share" ) {
    print OUT "share = \"$share\"\n";
    print OUT "sharebin = \"$share/bin\/${arch}\"\n";
}
if ( "$master_arch" ) {
    print OUT "master_arch = \"$master_arch\"\n";
}

close OUT;
