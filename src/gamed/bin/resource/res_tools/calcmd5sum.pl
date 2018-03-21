use Digest::file qw(digest_file_hex);
use Digest::MD5 qw(md5_hex);
use Config::IniFiles;

$stringMd5;
@fileList = ("./assets/gamedata/datatempl.gbd", 
	         "./assets/gamedata/skill.gbd",
	         "./assets/gamedata/task.gbd");

sub saveMd5toFile
{
    my ($md5string) = (@_);
    $md5file = "md5file";
    open VER, "> $md5file";
    print VER "$md5string";
    close VER;
}

for (@fileList) {
	@filepath = split('/', $_);
	$file = pop(@filepath);
	$fileMd5 = digest_file_hex($_, "MD5");

	print $fileMd5, " $file\n";

	$stringMd5 = $stringMd5."$file $fileMd5\n";
}

$versionMd5 = md5_hex($stringMd5);
print "$versionMd5\n";

my $cfg = Config::IniFiles->new(-file => "./assets/config/version.conf");
if ($cfg) {
	$cfg->setval("GameVersion", "resource_version", $versionMd5);
	$cfg->RewriteConfig();
	print "SUCCESS: version.conf update success! resource_version = $versionMd5\n";
}
else {
	print "ERROR: version.conf file not found! update failure.\n";
}
