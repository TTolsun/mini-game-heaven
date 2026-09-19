import fs from 'node:fs';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
import crypto from 'node:crypto';
import {fileURLToPath} from 'node:url';

export function validateRelease(tag, badging, certificates, expectedFingerprint, expectedPackage) {
  if (!/^v\d+\.\d+\.\d+$/.test(tag)) throw new Error('Release tag must be vMAJOR.MINOR.PATCH');
  const packageLine = badging.split('\n').find(line => line.startsWith('package:')) ?? '';
  const value = key => packageLine.match(new RegExp(`${key}='([^']+)'`))?.[1];
  if (!expectedPackage || value('name') !== expectedPackage) throw new Error('Wrong APK package');
  if (value('versionName') !== tag.slice(1)) throw new Error('Tag and APK version differ');
  if (!/^[1-9]\d*$/.test(value('versionCode') ?? '')) throw new Error('Invalid versionCode');
  if (/^application-debuggable\b/m.test(badging)) throw new Error('Debuggable APK cannot be released');
  const expected = expectedFingerprint?.replaceAll(':', '').toLowerCase();
  if (!/^[0-9a-f]{64}$/.test(expected ?? '')) throw new Error('RELEASE_CERT_SHA256 is required');
  const actual = [...certificates.matchAll(/^Signer #\d+ certificate SHA-256 digest:\s*([0-9a-f:]+)\s*$/gmi)]
    .map(m => m[1].replaceAll(':', '').toLowerCase());
  if (actual.length !== 1 || actual[0] !== expected) throw new Error('Release certificate mismatch');
  return {version: tag.slice(1), versionCode: Number(value('versionCode')), certificate: expected};
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  try {
    const [tag] = process.argv.slice(2);
    if (!/^v\d+\.\d+\.\d+$/.test(tag ?? '')) throw new Error('Use release.mjs vMAJOR.MINOR.PATCH');
    const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
    const apk = path.join(root, 'app/build/outputs/apk/release/app-release.apk');
    const buildTools = path.join(process.env.ANDROID_HOME ?? '', 'build-tools/36.0.0');
    const certificates = execFileSync(path.join(buildTools, 'apksigner'), ['verify', '--verbose', '--print-certs', apk], {encoding: 'utf8'});
    const badging = execFileSync(path.join(buildTools, 'aapt'), ['dump', 'badging', apk], {encoding: 'utf8'});
    const gradle = fs.readFileSync(path.join(root, 'app/build.gradle.kts'), 'utf8');
    const expectedPackage = gradle.match(/^\s*applicationId\s*=\s*"([^"]+)"/m)?.[1];
    const metadata = validateRelease(tag, badging, certificates, process.env.RELEASE_CERT_SHA256, expectedPackage);
    const notes = path.join(root, `docs/releases/${metadata.version}.md`);
    if (!fs.existsSync(notes) || fs.readFileSync(notes, 'utf8').trim().length < 80) throw new Error('Reviewed release notes are required');
    const dest = path.join(root, 'build/release');
    fs.mkdirSync(dest, {recursive: true});
    const name = `android-app-${metadata.version}.apk`;
    const data = fs.readFileSync(apk);
    fs.writeFileSync(path.join(dest, name), data);
    fs.writeFileSync(path.join(dest, 'SHA256SUMS.txt'), `${crypto.createHash('sha256').update(data).digest('hex')}  ${name}\n`);
    fs.writeFileSync(path.join(dest, 'verification.json'), JSON.stringify({
      ...metadata, sourceCommit: execFileSync('git', ['rev-parse', 'HEAD'], {cwd: root, encoding: 'utf8'}).trim(),
      review: JSON.parse(fs.readFileSync(path.join(root, 'docs/sdd-review.json'), 'utf8')),
    }, null, 2) + '\n');
  } catch (error) { console.error(error.message); process.exitCode = 1; }
}
