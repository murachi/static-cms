import { argv, exit } from "node:process";
import fsPromises from "node:fs/promises";
import path from "node:path";
import { simpleGit, SimpleGit, SimpleGitOptions } from "simple-git";
import { config } from "node-config-ts";

// コマンドライン解析
if (argv.length < 4) {
  console.log(`usage: node ${argv[1]} branch '[commit list...]'`);
  exit(1);
}

const branch = argv[2];
const commits = JSON.parse(argv[3]) as any[];

(async () => {
  // Git 操作
  try {
    await fsPromises.access(config.GitRepository.baseDir);
    // すでにディレクトリがある: ブランチを変更して最新化
    const git = simpleGit(config.GitRepository.baseDir);
    await git.fetch();
    await git.checkout(branch);
    await git.pull();
  }
  catch (error) {
    // ディレクトリがない: clone から実施
    fsPromises.mkdir(config.GitRepository.baseDir, { recursive: true });
    const git = simpleGit();
    await git.clone(config.GitRepository.remoteURL, config.GitRepository.baseDir);
    await git.cwd(config.GitRepository.baseDir);
    await git.checkout(branch);
  }

  // Webファイル展開
  const target_root = path.join(config.WebContents.baseDir, branch);
  try {
    await fsPromises.access(target_root);
  }
  catch (error) {
    fsPromises.mkdir(target_root, { recursive: true });
  }
  //TODO: Webファイル展開処理の続きを書く
})();
