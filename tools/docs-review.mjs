import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';

export const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const recordPath = 'docs/sdd-review.json';
const sourcePaths = ['native', 'app/src/main', 'app/build.gradle.kts', 'build.gradle.kts',
  'settings.gradle.kts', 'gradle.properties', 'gradle', 'sdd.yaml', 'docs/sdd-config',
  'tools/sdd-tool.json', 'tools/docs-review.mjs'];
const hash = value => crypto.createHash('sha256').update(value).digest('hex');
export function canonical(value) {
  if (Array.isArray(value)) return value.map(canonical);
  if (value && typeof value === 'object') return Object.fromEntries(
    Object.keys(value).sort().map(k => [k, canonical(value[k])]));
  return value;
}
export function normalizedFacts(value) {
  const facts = structuredClone(value);
  for (const k of ['generated_at', 'source_root', 'source_commit', 'compile_commands']) delete facts.meta?.[k];
  return canonical(facts);
}
function digestFile(file) {
  const data = fs.readFileSync(file);
  // Text checkout EOL changes must not invalidate a review on another OS.
  return hash(data.includes(0) ? data : data.toString('utf8').replace(/\r\n/g, '\n'));
}
function walk(dir) {
  return fs.readdirSync(dir, {withFileTypes: true}).flatMap(entry => {
    const file = path.join(dir, entry.name);
    if (entry.isSymbolicLink()) throw new Error(`Symlink is not reviewable: ${file}`);
    return entry.isDirectory() ? walk(file) : [file];
  });
}
export function snapshot(dir) {
  const names = execFileSync('git', ['ls-files', '-z', '--cached', '--others', '--exclude-standard', '--', ...sourcePaths],
    {cwd: dir, encoding: 'utf8'}).split('\0').filter(Boolean);
  const source = {};
  for (const name of [...new Set(names)].sort()) {
    const file = path.join(dir, name);
    if (!fs.existsSync(file)) continue; // Tracked deletion is reflected by the missing key.
    if (fs.lstatSync(file).isSymbolicLink()) throw new Error(`Symlink is not reviewable: ${name}`);
    source[name] = digestFile(file);
  }
  if (!Object.keys(source).some(n => n.startsWith('native/'))) throw new Error('No native sources found');
  const documents = {};
  const needsReview = [];
  for (const file of walk(path.join(dir, 'docs/sdd')).sort()) {
    const name = path.relative(dir, file).replaceAll('\\', '/');
    if (!/\.(md|mmd|json)$/.test(name)) continue;
    documents[name] = name.endsWith('/facts.json')
      ? hash(JSON.stringify(normalizedFacts(JSON.parse(fs.readFileSync(file, 'utf8'))))) : digestFile(file);
    if (name.endsWith('.md')) {
      const text = fs.readFileSync(file, 'utf8').replace(/\r\n/g, '\n');
      const fm = text.match(/^---\n([\s\S]*?)\n---(?:\n|$)/)?.[1];
      const status = fm?.match(/^status:\s*(\S+)\s*$/m)?.[1];
      if (fm && status !== 'ok' && status !== 'needs-review') throw new Error(`Invalid review status: ${name}`);
      if (status === 'needs-review') needsReview.push(name);
    }
  }
  if (!documents['docs/sdd/facts.json'] || !documents['docs/sdd/index.md']) throw new Error('SDD is incomplete');
  return {source, documents, needsReview};
}
export function accept(dir, reviewer, acknowledgements = {}, reason = '') {
  if (!reviewer?.trim() || !reason?.trim()) throw new Error('--reviewer and --reason are required');
  const current = snapshot(dir);
  for (const name of current.needsReview) {
    if (!acknowledgements[name]?.trim()) throw new Error(`Review or explicitly acknowledge the limitation: ${name}`);
  }
  for (const name of Object.keys(acknowledgements)) {
    if (!current.needsReview.includes(name)) throw new Error(`Not a needs-review page: ${name}`);
  }
  const record = {version: 1, reviewer: reviewer.trim(), reviewedAt: new Date().toISOString(),
    reason: reason.trim(), acknowledgements, ...current};
  fs.writeFileSync(path.join(dir, recordPath), JSON.stringify(record, null, 2) + '\n');
  return record;
}
export function check(dir) {
  const file = path.join(dir, recordPath);
  if (!fs.existsSync(file)) throw new Error('No review record. Review the SDD and run docs-review.mjs accept.');
  const record = JSON.parse(fs.readFileSync(file, 'utf8'));
  if (record.version !== 1 || !record.reviewer?.trim() || !record.reason?.trim() ||
      !Number.isFinite(Date.parse(record.reviewedAt))) throw new Error('Review record is pending or invalid');
  const current = snapshot(dir);
  const changes = [];
  for (const kind of ['source', 'documents']) {
    for (const name of new Set([...Object.keys(record[kind] ?? {}), ...Object.keys(current[kind])])) {
      if (record[kind]?.[name] !== current[kind][name]) changes.push(name);
    }
  }
  if (changes.length) throw new Error(`Review is stale:\n${changes.join('\n')}`);
  for (const name of current.needsReview) {
    if (!record.acknowledgements?.[name]?.trim()) throw new Error(`Unreviewed limitation: ${name}`);
  }
  return record;
}
if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  try {
    const [command, ...args] = process.argv.slice(2);
    if (command === 'check' && !args.length) {
      const record = check(root);
      console.log(`Review current: ${record.reviewer} (${record.reviewedAt})`);
    } else if (command === 'accept') {
      if (process.env.CI || process.env.GITHUB_ACTIONS) throw new Error('CI must not create review records');
      let reviewer, reason;
      const acknowledgements = {};
      for (let i = 0; i < args.length; i += 2) {
        const value = args[i + 1];
        if (!value) throw new Error('Missing option value');
        if (args[i] === '--reviewer') reviewer = value;
        else if (args[i] === '--reason') reason = value;
        else if (args[i] === '--acknowledge') {
          const at = value.indexOf('=');
          if (at < 1) throw new Error('Use --acknowledge docs/sdd/page.md=reason');
          acknowledgements[value.slice(0, at)] = value.slice(at + 1);
        } else throw new Error(`Unknown option: ${args[i]}`);
      }
      accept(root, reviewer, acknowledgements, reason);
      console.log('Review recorded. Run tools/docs-check.sh before committing.');
    } else throw new Error('Usage: docs-review.mjs check | accept --reviewer NAME --reason TEXT [--acknowledge PAGE=REASON]');
  } catch (error) { console.error(error.message); process.exitCode = 1; }
}
