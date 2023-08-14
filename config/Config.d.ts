/* tslint:disable */
/* eslint-disable */
declare module "node-config-ts" {
  interface IConfig {
    WebHookSignature: WebHookSignature
    GitRepository: GitRepository
    WebContents: WebContents
  }
  interface WebContents {
    baseDir: string
  }
  interface GitRepository {
    baseDir: string
    remoteURL: string
  }
  interface WebHookSignature {
    secret: string
  }
  export const config: Config
  export type Config = IConfig
}
