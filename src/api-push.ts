/** GitHub の push イベント処理 */
import crypto from "node:crypto";
import { IncomingHttpHeaders } from "node:http";
import { fork } from "node:child_process";
import { config } from "node-config-ts";

const responseMessages = {
  ok: { status: "Ok", message: "処理を実行しました。" },
  invalidAccess: { status: "Error", message: "リクエストが不正です。" },
  disagreementSignature: { status: "Error", message: "シークレットコードが一致しないため、処理を継続できません。" },
  notSupportedEvent: { status: "Ok", message: "未対応のイベントのため、処理しません。" },
  identifyBranchFailed: { status: "Error", message: "ブランチの特定に失敗しました。" }
};

namespace API_push {
  export type Result = {
    statusCode: number,
    resultBody: {
      status: string,
      message: string
    }
  };

  /** GitHub push イベント受付APIの処理 */
  export function handler(headers: IncomingHttpHeaders, body: string) : Result {
    // シグニチャを確認
    if (!("x-hub-signature-256" in headers)) {
      return {
        statusCode: 400,
        resultBody: responseMessages.invalidAccess
      };
    }
    const hmac = crypto.createHmac("sha256", config.WebHookSignature.secret as string);
    hmac.update(body);
    if (headers["x-hub-signature-256"] !== `sha256=${hmac.digest('hex')}`) {
      return {
        statusCode: 403,
        resultBody: responseMessages.disagreementSignature
      };
    }
    // pushイベントであることを確認
    if (!("x-github-event" in headers)) {
      return {
        statusCode: 400,
        resultBody: responseMessages.invalidAccess
      };
    }
    if (headers["x-github-event"] !== "push") {
      return {
        statusCode: 200,
        resultBody: responseMessages.notSupportedEvent
      };
    }
    // ペイロードをJSONとして解析
    let data;
    try {
      data = JSON.parse(body);
    }
    catch (error) {
      return {
        statusCode: 400,
        resultBody: responseMessages.invalidAccess
      };
    }
    // ブランチを取得
    if (!("ref" in data && "commits" in data)) {
      return {
        statusCode: 400,
        resultBody: responseMessages.invalidAccess
      };
    }
    const branch_match = (data.ref as string).match(/^refs\/heads\/(.+)$/);
    if (branch_match == null) {
      return {
        statusCode: 400,
        resultBody: responseMessages.identifyBranchFailed
      };
    }
    const branch = branch_match[1];
    // コミット内容を取得
    const commits = data.commits as any[];

    // git操作およびWebファイル展開の処理プロセスを実行する
    fork("exec/deploy-webfiles.js", [branch, JSON.stringify(commits)]);

    return {
      statusCode: 200,
      resultBody: responseMessages.ok
    };
  }
}

export default API_push;
