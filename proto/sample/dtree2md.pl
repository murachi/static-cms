use strict;
use List::Util qw/any/;
use File::Path qw/make_path/;
use File::Copy;

my $MD_FILENAME_FORMAT = 'file-%s.%s';

my %ext2fence = (
  cpp => sub { $_[0]->{'c++'} = 1; 'c++' },
  h => sub { exists $_[0]->{'c++'} ? 'c++' :'c' },
  csproj => sub { 'xml' },
  cs => sub { 'c#' },
  stetic => sub { 'xml' },
  pl => sub { 'perl' },
  py => sub { 'python' },
  rs => sub { 'rust' },
  c => sub { 'c' },
  html => sub { 'html' },
  config => sub { 'xml' },
  xsd => sub { 'xml' },
  resx => sub { 'xml' },
  settings => sub { 'xml' },
  xaml => sub { 'xml' },
  md => sub { 'markdown' },
  scss => sub { 'css' },
  sass => sub { 'css' },
  ini => sub { 'ini' },
  svg => sub { 'xml' },
  sh => sub { 'sh' },
  ts => sub { 'typescript' },
  js => sub { 'javascript' },
  json => sub { 'json' },
  yml => sub { 'yaml' },
  vue => sub { 'html' },
  php => sub { 'php' },
  toml => sub { 'toml' },
);

my @pat2fence = (
  { pat => qr/\.ini\.sample$/, fence => 'ini' },
),

my %fn2fence = (
  Pipfile => sub { 'toml' },
  'Pipfile.lock' => sub { 'json' },
  'Cargo.lock' => sub { 'toml' },
  'composer.lock' => sub { 'json' },
  '.babelrc' => sub { 'json' },
  '.editorconfig' => sub { 'toml' },
  '.prettierrc' => sub { 'json' },
  'Vagrantfile' => sub { 'ruby' },
);

my %picture_ext = qw(png 1 jpg 1 jpeg 1 gif 1 ico 1 svg 1 webp 1);

my %binary_ext = qw(sdf 1 keepme 1);

sub walk {
  my ($src, $dist, $distroot, $ignore_pattern) = @_;
  $distroot = $distroot || $dist;
  opendir my $dh, $src or die "ディレクトリ $src を開けません";
  unless (-d $dist) {
    make_path($dist) or die "ディレクトリ $dist を作成できません";
  }
  # '.' と '..' は除外。拡張子の辞書順でソート。
  my @items = sort {
    my ($xt, $xe) = $a =~ /^(.*?)(?:\.([^\.]+))?$/;
    my ($yt, $ye) = $b =~ /^(.*?)(?:\.([^\.]+))?$/;
    $xe cmp $ye || $xt cmp $yt
  } grep { !/^\.+$/ } readdir $dh;
  # .gitignore があれば情報を収集
  if (any { $_ eq '.gitignore' } @items) {
    open my $fin, "<$src/.gitignore" or die "ファイル $src/.gitignore を開けません";
    my @patterns = <$fin>;
    chomp @patterns;
    push @$ignore_pattern, { base => $src, patterns => [@patterns] };
  }

  my ($dir_stat, @dist_items) = ({});
  ITEMS_LOOP:
  for my $file (@items) {
    my $item = "$src/$file";
    # .gitignore 適用対象を除外
    for my $igpat (@$ignore_pattern) {
      next ITEMS_LOOP if is_ignore($item, @$igpat{qw(base patterns)});
    }
    # ディレクトリなら再帰
    if (-d $item) {
      walk($item, "$dist/$file", $distroot, $ignore_pattern);
      push @dist_items, ["$file", 'd'];
      next;
    }
    push @dist_items, dispose_file($file, $src, $dist, $dir_stat);
  }

  open my $fout, ">$dist/index.md" or die "ファイル $dist/index.md を作成できません";
  print $fout "# $src ディレクトリ\n\n";
  for my $elem (@dist_items) {
    my ($file, $type) = @$elem;
    if ($type eq 'd') { print $fout "- [$file/]($file/index.html)\n"; }
    elsif ($type eq 'md') {
      my $htmlfile = $file;
      $htmlfile =~ s/md$/html/i;
      print $fout "- [$file]($htmlfile)\n";
    }
    else {
      my $htmlfile = sprintf $MD_FILENAME_FORMAT, $file, 'html';
      print $fout "- [$file]($htmlfile)\n";
    }
  }
}

sub is_ignore {
  my ($item, $base, $patterns) = @_;
  my $is_ignore = 0;
  for my $pattern (@$patterns) {
    my $pat = "$pattern";
    my $hit_flag = !($pat =~ /^\~/);
    $pat = $hit_flag ? $pat : substr $pat, 1;
    $pat =~ s/\./\\./g;
    $pat =~ s/\*/.*?/g;
    $pat =~ s!/$!!;
    if ($pat =~ m!^/!) {
      $is_ignore = $item =~ m!^\Q$base\E$pat(/.*|$)! ? $hit_flag : $is_ignore;
    }
    else {
      $is_ignore = $item =~ m!^\Q$base\E/(.*?/)?$pat(/.*|$)! ? $hit_flag : $is_ignore;
    }
  }

  $is_ignore
}

sub dispose_file {
  my ($file, $src, $dist, $dir_stat) = @_;
  my $item = "$src/$file";
  my ($ext) = $file =~ /\.([^\.]*)$/ or ('');
  # Markdown ファイルはそのままコピー
  if ($ext eq 'md') {
    copy($item, "$dist/$file") or die "$item -> $dist/$file のコピーに失敗";
    return ["$file", 'md'];
  }
  # 画像ファイルの場合は、画像を埋め込む Markdown を生成
  return copy_picture_with_markdown($file, $src, $dist) if exists $picture_ext{$ext};
  # バイナリファイルの場合は、リンクを含む Markdown を生成
  return copy_binary_with_markdown($file, $src, $dist) if exists $binary_ext{$ext};
  # それ以外の場合、ソースを埋め込んだ Markdown を生成
  return write_markdown($file, $src, $dist, $dir_stat);
}

sub write_markdown {
  my ($file, $src, $dist, $dir_stat) = @_;
  my $item = "$src/$file";
  my ($ext) = $file =~ /\.([^\.]*)$/ or ('');
  # fence 情報を決める
  my $fence = exists $fn2fence{$file} ? $fn2fence{$file}->() : '';
  unless ($fence) {
    for my $p2f (@pat2fence) {
      my ($pat, $fc) = @$p2f{qw(pat fence)};
      $fence = $fc, last if $file =~ /$pat/;
    }
  }
  $fence = $fence || (exists $ext2fence{$ext} ? $ext2fence{$ext}->($dir_stat) : '');
  # Markdown として出力
  open my $fin, "<$item" or die "ファイル $item を開けません";
  my $distfile = "$dist/" . sprintf($MD_FILENAME_FORMAT, $file, 'md');
  open my $fout, ">$distfile" or die "ファイル $distfile を開けません";
  print $fout <<ENDLINE;
# `$file`

- ファイルの種類: `$fence`
- ファイルの拡張子: `$ext`
- ファイルサイズ: ${\(-s $item)}

```$fence
ENDLINE
  my $empty_lines = 0;
  while (<$fin>) {
    chomp;
    my $is_empty = /^\s*$/;
    print $fout "$_\n" unless $empty_lines && $is_empty;
    $empty_lines = $is_empty;
  }
  print $fout "```\n";

  [$file, 'f']
}

sub copy_picture_with_markdown {
  my ($file, $src, $dist) = @_;
  my $item = "$src/$file";
  copy($item, "$dist/$file") or die "ファイル $item を $dist へコピーできません";
  # Markdown 作成
  my $distfile = "$dist/" . sprintf($MD_FILENAME_FORMAT, $file, 'md');
  open my $fout, ">$distfile" or die "ファイル $distfile を開けません";
  print $fout <<ENDLINE;
# `$file`

- ファイルの種類: 画像ファイル
- ファイルサイズ: ${\(-s $item)}

![$file]($file)
ENDLINE

  [$file, 'f']
}

sub copy_binary_with_markdown {
  my ($file, $src, $dist) = @_;
  my $item = "$src/$file";
  copy($item, "$dist/$file") or die "ファイル $item を $dist へコピーできません";
  # Markdown 作成
  my $distfile = "$dist/" . sprintf($MD_FILENAME_FORMAT, $file, 'md');
  open my $fout, ">$distfile" or die "ファイル $distfile を開けません";
  print $fout <<ENDLINE;
# `$file`

- ファイルの種類: その他バイナリ
- ファイルサイズ: ${\(-s $item)}

**[ダウンロード]($file)**
ENDLINE

  [$file, 'f']
}

my ($src, $dist) = @ARGV;
unless ($src) {
  print STDERR ("収集するソースツリーがあるディレクトリを指定してください。\n");
  exit 1;
}
unless ($dist) {
  print STDERR ("出力先ディレクトリを指定してください。\n");
  exit 1;
}
$src =~ s!/$!!;
$dist =~ s!/$!!;

unless (-d $src) {
  print STDERR ("存在するディレクトリを指定してください。\n");
  exit 1;
}

walk($src, $dist, undef, [{ base => $src, patterns => ['.git/'] }]);
