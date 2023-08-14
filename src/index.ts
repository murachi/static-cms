/** Static CMS エントリポイント */
import express from "express";
import bodyParser from "body-parser";
import API_push from "./api-push.js";
import { config } from "node-config-ts";

const responseMessages = {
  invalidAccess: { status: "Error", message: "リクエストが不正です。" },
  disagreementSignature: { status: "Error", message: "シークレットコードが一致しないため、処理を継続できません。" },
  notSupportedEvent: { status: "Ok", message: "未対応のイベントのため、処理しません。" },
  identifyBranchFailed: { status: "Error", message: "ブランチの特定に失敗しました。" }
};

const port = 3000;
const app = express();

app.use(bodyParser.text({type: '*/*', defaultCharset: 'utf-8'}));

/** ルーティング */
app.post("/api/push", (req, res) => {
  const api_result = API_push.handler(req.headers, req.body);
  res.status(api_result.statusCode);
  res.json(api_result.resultBody);
});

app.listen(port, () => {

});